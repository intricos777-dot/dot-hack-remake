#include "LiaBridge.hpp"
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

std::map<std::string, std::string> LiaBridge::parseLine(const std::string& line) {
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
    i++;
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
      // Unescape
      std::string unesc;
      for (size_t j = 0; j < raw.size(); j++) {
        if (raw[j] == '\\' && j + 1 < raw.size()) {
          char nxt = raw[j + 1];
          if (nxt == '"') unesc += '"';
          else if (nxt == '\\') unesc += '\\';
          else if (nxt == 'n') unesc += '\n';
          else if (nxt == 't') unesc += '\t';
          else unesc += nxt;
          j++;
        } else {
          unesc += raw[j];
        }
      }
      val = unesc;
    } else {
      while (i < n && line[i] != ',' && line[i] != '}') val += line[i++];
    }
    out[key] = val;
  }
  return out;
}

bool LiaBridge::start(const std::string& python, const std::string& script) {
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
  rd_ = std::thread([this] { readerLoop(); });
  return true;
}

void LiaBridge::writeLine(const std::string& line) {
  if (!running_) return;
  std::string msg = line + "\n";
  size_t off = 0;
  while (off < msg.size()) {
    ssize_t w = write(inFd_, msg.data() + off, msg.size() - off);
    if (w < 0) { if (errno == EINTR) continue; return; }
    off += (size_t)w;
  }
}

void LiaBridge::readerLoop() {
  std::string buf;
  char chunk[4096];
  while (running_) {
    struct pollfd pfd{outFd_, POLLIN, 0};
    int pr = ::poll(&pfd, 1, 50);
    if (pr < 0) break;
    if (pr == 0) continue;
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
        auto parsed = parseLine(line);
        if (!parsed.empty()) {
          std::lock_guard<std::mutex> g(mu_);
          q_.push_back(parsed);
        }
      }
    }
    if (pfd.revents & POLLHUP) break;
  }
  alive_ = false;
}

std::vector<std::map<std::string, std::string>> LiaBridge::poll() {
  std::vector<std::map<std::string, std::string>> out;
  std::lock_guard<std::mutex> g(mu_);
  while (!q_.empty()) { out.push_back(q_.front()); q_.pop_front(); }
  return out;
}

void LiaBridge::createTask(const std::string& project, const std::string& title, const std::string& context) {
  writeLine("{\"type\":\"task\",\"project\":\"" + project + "\",\"title\":\"" + title + "\",\"context\":\"" + context + "\"}");
}

void LiaBridge::queryTasks(const std::string& project) {
  writeLine("{\"type\":\"task_query\",\"project\":\"" + project + "\"}");
}

void LiaBridge::updateTask(const std::string& id, const std::string& action, const std::string& result) {
  writeLine("{\"type\":\"task_update\",\"id\":\"" + id + "\",\"action\":\"" + action + "\",\"result\":\"" + result + "\"}");
}

void LiaBridge::getSummary(const std::string& project) {
  writeLine("{\"type\":\"task_summary\",\"project\":\"" + project + "\"}");
}

void LiaBridge::stop() {
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
