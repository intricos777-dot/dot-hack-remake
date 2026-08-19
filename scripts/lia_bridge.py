#!/usr/bin/env python3
"""Lia.CLI task bridge for .hack remake.
Runs alongside aida_bridge.py, exposing Lia.CLI's supervisor/task-tracker
through the same JSON-lines-over-stdio protocol.

Protocol:
  Game → Bridge: {"type": "task", "project": "...", "title": "...", "context": "..."}
  Bridge → Game: {"type": "task_created", "id": "task-N", "project": "...", "title": "..."}
  Game → Bridge: {"type": "task_query", "project": "..."}
  Bridge → Game: {"type": "task_list", "tasks": [...]}
  Game → Bridge: {"type": "task_update", "id": "task-N", "action": "start|complete|fail", "result": "..."}
  Bridge → Game: {"type": "task_updated", "id": "task-N", "status": "..."}
"""

import json
import os
import re
import subprocess
import sys
import time
import traceback
from pathlib import Path

LIA_CLI = os.environ.get("LIA_CLI", str(Path.home() / "/home/sin/Projects/lia-cli/src/index.ts"))
PLAYERS_DIR = Path(os.environ.get("MINE_OS_PLAYERS", Path.home() / ".mine_os" / "players"))


class LiaTaskBridge:
    """Bridge Lia.CLI supervisor to the Aida JSON protocol."""

    def __init__(self):
        self.tasks_dir = PLAYERS_DIR / "supervisor"
        self.tasks_dir.mkdir(parents=True, exist_ok=True)
        self.tasks_file = self.tasks_dir / "tasks.json"
        self.tasks = {}
        self.next_id = 1
        self._load()

    def _load(self):
        try:
            if self.tasks_file.exists():
                data = json.loads(self.tasks_file.read_text())
                self.tasks = data.get("tasks", {})
                self.next_id = data.get("next_id", 1)
        except Exception:
            self.tasks = {}
            self.next_id = 1

    def _save(self):
        self.tasks_file.write_text(json.dumps({
            "tasks": self.tasks,
            "next_id": self.next_id
        }, indent=2))

    def handle(self, msg):
        t = msg.get("type")

        if t == "task":
            return self._create_task(msg)
        elif t == "task_query":
            return self._query_tasks(msg)
        elif t == "task_update":
            return self._update_task(msg)
        elif t == "task_summary":
            return self._summary(msg)
        else:
            return {"type": "error", "message": f"unknown type: {t}"}

    def _create_task(self, msg):
        project = msg.get("project", "dot-hack-remake")
        title = msg.get("title", "untitled task")
        context = msg.get("context", "")
        task_id = f"task-{self.next_id}"
        self.next_id += 1
        now = time.strftime("%Y-%m-%dT%H:%M:%S")
        self.tasks[task_id] = {
            "id": task_id,
            "project": project,
            "title": title,
            "context": context,
            "status": "pending",
            "assignee": None,
            "result": None,
            "created_at": now,
            "updated_at": now
        }
        self._save()
        return {"type": "task_created", "id": task_id, "project": project, "title": title, "status": "pending"}

    def _query_tasks(self, msg):
        project = msg.get("project")
        if project:
            tasks = [t for t in self.tasks.values() if t.get("project") == project]
        else:
            tasks = list(self.tasks.values())
        return {"type": "task_list", "tasks": tasks, "count": len(tasks)}

    def _update_task(self, msg):
        task_id = msg.get("id")
        action = msg.get("action", "start")
        result = msg.get("result", "")

        if task_id not in self.tasks:
            return {"type": "error", "message": f"unknown task: {task_id}"}

        task = self.tasks[task_id]
        now = time.strftime("%Y-%m-%dT%H:%M:%S")

        if action == "start":
            task["status"] = "in_progress"
            task["assignee"] = msg.get("assignee", "hermes-subagent")
        elif action == "complete":
            task["status"] = "completed"
            task["result"] = result
        elif action == "fail":
            task["status"] = "failed"
            task["result"] = result
        else:
            return {"type": "error", "message": f"unknown action: {action}"}

        task["updated_at"] = now
        self._save()
        return {"type": "task_updated", "id": task_id, "status": task["status"]}

    def _summary(self, msg):
        project = msg.get("project")
        if project:
            tasks = [t for t in self.tasks.values() if t.get("project") == project]
        else:
            tasks = list(self.tasks.values())
        pending = sum(1 for t in tasks if t["status"] == "pending")
        in_progress = sum(1 for t in tasks if t["status"] == "in_progress")
        completed = sum(1 for t in tasks if t["status"] == "completed")
        failed = sum(1 for t in tasks if t["status"] == "failed")
        return {
            "type": "task_summary",
            "total": len(tasks),
            "pending": pending,
            "in_progress": in_progress,
            "completed": completed,
            "failed": failed,
            "tasks": tasks
        }


def main():
    bridge = LiaTaskBridge()
    sys.stdout.write(json.dumps({"type": "status", "text": "Lia task bridge online"}) + "\n")
    sys.stdout.flush()

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            msg = json.loads(line)
            response = bridge.handle(msg)
            sys.stdout.write(json.dumps(response, ensure_ascii=False) + "\n")
            sys.stdout.flush()
        except Exception:
            sys.stderr.write("lia bridge error: " + traceback.format_exc())
            sys.stderr.flush()
            sys.stdout.write(json.dumps({"type": "error", "message": "internal error"}) + "\n")
            sys.stdout.flush()


if __name__ == "__main__":
    main()
