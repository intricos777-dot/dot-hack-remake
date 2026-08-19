#include "Bridge.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <signal.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace aida {

static void setNonBlocking(int fd) {
  int fl = fcntl(fd, F_GETFL, 0);
  if (fl >= 0) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

std::string Bridge::unescape(const std::string& s) {
  std::string o;
  size_t i = 0;
  while (i < s.size()) {
    char c = s[i];
    if (c == '\\' && i + 1 < s.size()) {
      char n = s[i + 1];
      switch (n) {
        case '"': o += '"'; i += 2; continue;
        case '\\': o += '\\'; i += 2; continue;
        case 'n': o += '\n'; i += 2; continue;
        case 't': o += '\t'; i += 2; continue;
        case 'r': o += '\r'; i += 2; continue;
        case 'u': {
          std::string hex;
          size_t j = i + 2;
          while (j < s.size() && hex.size() < 4 && isxdigit((unsigned char)s[j])) hex += s[j++];
          if (hex.size() == 4) {
            unsigned cp = (unsigned)strtoul(hex.c_str(), nullptr, 16);
            if (cp < 0x80) o += (char)cp;
            else if (cp < 0x800) { o += (char)(0xC0 | (cp >> 6)); o += (char)(0x80 | (cp & 0x3F)); }
            else { o += (char)(0xE0 | (cp >> 12)); o += (char)(0x80 | ((cp >> 6) & 0x3F)); o += (char)(0x80 | (cp & 0x3F)); }
            i = j;
            continue;
          }
          o += '?'; i += 2; continue;
        }
        default: o += n; i += 2; continue;
      }
    }
    o += c;
    i++;
  }
  return o;
}

std::map<std::string, std::string> Bridge::parseLine(const std::string& line) {
  std::map<std::string, std::string> out;
  size_t i = 0;
  const size_t n = line.size();
  while (i < n) {
    while (i < n && (line[i] == ' ' || line[i] == '{' || line[i] == ',' || line[i] == '\t' || line[i] == '}')) i++;
    if (i >= n || line[i] != '"') { if (i < n) i++; continue; }
    size_t k0 = ++i;
    while (i < n && line[i] != '"') i++;
    if (i >= n) break;
    std::string key = line.substr(k0, i - k0);
    i++; // past closing quote
    while (i < n && (line[i] == ' ' || line[i] == ':')) i++;
    std::string val;
    if (i < n && line[i] == '"') {
      i++;
      std::string raw;
      while (i < n) {
        if (line[i] == '\\' && i + 1 < n) { raw += line[i]; raw += line[i + 1]; i += 2; continue; }
        if (line[i] == '"') { i++; break; }
        raw += line[i]; i++;
      }
      val = unescape(raw);
    } else {
      while (i < n && line[i] != ',' && line[i] != '}') val += line[i++];
    }
    out[key] = val;
  }
  return out;
}

bool Bridge::start(const std::string& python, const std::string& script, const std::string& personaJson) {
  int inPipe[2], outPipe[2];
  if (pipe(inPipe) != 0 || pipe(outPipe) != 0) return false;
  pid_ = fork();
  if (pid_ < 0) return false;
  if (pid_ == 0) {
    dup2(inPipe[0], STDIN_FILENO);
    dup2(outPipe[1], STDOUT_FILENO);
    close(inPipe[0]); close(inPipe[1]); close(outPipe[0]); close(outPipe[1]);
    execl(python.c_str(), python.c_str(), "-u", script.c_str(), nullptr);
    _exit(127);
  }
  inFd_ = inPipe[1];
  outFd_ = outPipe[0];
  close(inPipe[0]);
  close(outPipe[1]);
  setNonBlocking(outFd_);
  running_ = true;
  alive_ = true;
  writeLine("{\"type\":\"init\",\"persona\":" + personaJson + "}");
  rd_ = std::thread([this] { readerLoop(); });
  return true;
}

void Bridge::writeLine(const std::string& line) {
  if (!running_) return;
  std::string msg = line + "\n";
  size_t off = 0;
  while (off < msg.size()) {
    ssize_t w = write(inFd_, msg.data() + off, msg.size() - off);
    if (w < 0) { if (errno == EINTR) continue; return; }
    off += (size_t)w;
  }
}

void Bridge::readerLoop() {
  std::string buf;
  char chunk[4096];
  while (running_) {
    struct pollfd pfd{outFd_, POLLIN, 0};
    int pr = ::poll(&pfd, 1, 50);
    if (pr < 0) break;
    if (pr == 0) continue;
    if (pfd.revents & (POLLHUP | POLLERR)) {
      // drain what's left
    }
    if (pfd.revents & POLLIN) {
      ssize_t n = read(outFd_, chunk, sizeof(chunk));
      if (n < 0) { if (errno == EINTR || errno == EAGAIN) continue; break; }
      if (n == 0) break;
      buf.append(chunk, (size_t)n);
      size_t nl;
      while ((nl = buf.find('\n')) != std::string::npos) {
        std::string line = buf.substr(0, nl);
        buf.erase(0, nl + 1);
        if (line.empty()) continue;
        auto f = parseLine(line);
        std::string type = f.count("type") ? f["type"] : "reply";
        Msg m;
        m.seq = seq_++;
        if (type == "reply") {
          m.kind = Msg::Reply;
          m.text = f.count("text") ? f["text"] : "";
        } else if (type == "status") {
          m.kind = Msg::Status;
          m.text = f.count("text") ? f["text"] : "";
        } else if (type == "action") {
          m.kind = Msg::Action_;
          m.id = f.count("id") ? strtoull(f["id"].c_str(), nullptr, 10) : 0;
          m.action.cmd = f.count("cmd") ? f["cmd"] : "";
          for (auto& [k, v] : f)
            if (k.rfind("arg_", 0) == 0) m.action.args[k.substr(4)] = v;
        } else if (type == "story") {
          m.kind = Msg::Story_;
          m.story.valid = true;
          m.story.arcTitle = f.count("arc_title") ? f["arc_title"] : m.story.arcTitle;
          m.story.arcSummary = f.count("arc_summary") ? f["arc_summary"] : m.story.arcSummary;
          m.story.quest = f.count("quest") ? f["quest"] : m.story.quest;
          auto items = [&](const std::string& pfx, std::vector<StoryItem>& out) {
            for (int i = 0; i < 32; i++) {
              std::string sk = pfx + std::to_string(i) + "_subject";
              std::string bk = pfx + std::to_string(i) + "_body";
              if (f.count(sk) && f.count(bk)) out.push_back({f[sk], f[bk]});
            }
          };
          items("news_", m.story.news);
          items("bbs_", m.story.bbs);
          items("mail_", m.story.mail);
        } else continue;
        {
          std::lock_guard<std::mutex> g(mu_);
          q_.push_back(m);
        }
      }
    }
    if (pfd.revents & POLLHUP) break;
  }
  alive_ = false;
}

std::vector<Msg> Bridge::poll() {
  std::vector<Msg> out;
  std::lock_guard<std::mutex> g(mu_);
  while (!q_.empty()) { out.push_back(q_.front()); q_.pop_front(); }
  return out;
}

void Bridge::sendUser(const std::string& text) {
  std::string t;
  for (char c : text) { if (c == '"' || c == '\\') t += '\\'; t += c; }
  writeLine("{\"type\":\"user\",\"text\":\"" + t + "\"}");
}

void Bridge::sendState(const GameState& st, bool notify) {
  writeLine("{\"type\":\"state\",\"notify\":" + std::string(notify ? "true" : "false") + ",\"world\":" + st.toJson() + "}");
}

void Bridge::sendPlayer(const std::string& name, bool ask) {
  std::string n;
  for (char c : name) { if (c == '"' || c == '\\') n += '\\'; n += c; }
  writeLine("{\"type\":\"player\",\"name\":\"" + n + "\",\"ask\":" + std::string(ask ? "true" : "false") + "}");
}

void Bridge::sendStoryRequest() {
  writeLine("{\"type\":\"story_request\"}");
}

void Bridge::sendActionResult(uint64_t id, bool ok, const std::string& note) {
  writeLine("{\"type\":\"action_result\",\"id\":" + std::to_string(id) + ",\"ok\":" + std::string(ok ? "true" : "false") + ",\"note\":\"" + note + "\"}");
}

void Bridge::stop() {
  if (!running_) return;
  running_ = false;
  if (rd_.joinable()) rd_.join();
  if (inFd_ >= 0) { close(inFd_); inFd_ = -1; }
  if (outFd_ >= 0) { close(outFd_); outFd_ = -1; }
  if (pid_ > 0) {
    kill(pid_, SIGTERM);
    for (int i = 0; i < 20; i++) {
      int st = 0;
      if (waitpid(pid_, &st, WNOHANG) == pid_) { pid_ = -1; break; }
      usleep(50000);
    }
    if (pid_ > 0) { kill(pid_, SIGKILL); waitpid(pid_, nullptr, 0); pid_ = -1; }
  }
}

} // namespace aida
