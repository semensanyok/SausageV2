#include "Logging.h"

using namespace std;

static ofstream log_file_stream("LOG.log");

static struct LogEntry {
  string message;
  std::source_location location;
};

static ThreadSafeQueue<LogEntry> log_queue;

// make stderr/stdout switch, separate methods or flag
// (stderr doesnt print immediately, bufferring output until OS decides to flush. good for perf, bad for emergency/crashes)
void LOG(const ostringstream& s,
         const std::source_location& location) {
  LOG(s.str(), location);
}

void LOG(const string& s, const std::source_location& location) {
  log_queue.Push({ s, location });
}

static void _LogMessage(ostream& stream, std::source_location& location, string& message) {
  stream
    << "["
    << location.file_name() << "("
    << location.line() << ":"
    << location.column() << ")#"
    << location.function_name()
    << "]"
    << message
    << endl;
}
namespace Sausage {
  void LogIO() {
    //this_thread::sleep_for(std::chrono::milliseconds(300));
    auto logs = log_queue.PopAll();
    while (!logs.empty()) {
      auto& log = logs.front();
      auto& location = log.location;
      auto& message = log.message;

      _LogMessage(cerr, location, message);
      //_LogMessage(cout, location, message);
      _LogMessage(log_file_stream, location, message);
      logs.pop();
    }
  }

  void WriteShaderMsgsToLogFile()
  {
    GLint maxMsgLen = 100000;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);
    int numMsgs = 1000;
    std::vector<GLchar> msgData(numMsgs * maxMsgLen);
    std::vector<GLenum> sources(numMsgs);
    std::vector<GLenum> types(numMsgs);
    std::vector<GLenum> severities(numMsgs);
    std::vector<GLuint> ids(numMsgs);
    std::vector<GLsizei> lengths(numMsgs);

    GLuint numFound = glGetDebugMessageLog(numMsgs, maxMsgLen, &sources[0], &types[0], &ids[0], &severities[0], &lengths[0], &msgData[0]);
    ofstream out("shader.log", ios::out);
    while (numFound > 0) {

      sources.resize(numFound);
      types.resize(numFound);
      severities.resize(numFound);
      ids.resize(numFound);
      lengths.resize(numFound);

      std::vector<std::string> messages;
      messages.reserve(numFound);

      std::vector<GLchar>::iterator currPos = msgData.begin();
      for (size_t msg = 0; msg < lengths.size(); ++msg)
      {
        //messages.push_back(std::string(currPos, currPos + lengths[msg] - 1));
        out << " MESSAGE: " << string(currPos, currPos + lengths[msg] - 1) << endl;
        currPos = currPos + lengths[msg];
      }
      numFound = glGetDebugMessageLog(numMsgs, messages.size(), &sources[0], &types[0], &ids[0], &severities[0], &lengths[0], &msgData[0]);
    }
  }
}
