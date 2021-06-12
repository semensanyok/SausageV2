#include "Logging.h"
static ofstream logstream("LOG.log");

static ThreadSafeQueue<string> log_queue;

void LOG(const string& s) {
	log_queue.Push(s);
}

thread LogIO(bool& quit) {
	return thread([&] {while (!quit) {
		//this_thread::sleep_for(std::chrono::milliseconds(300));
		auto s = log_queue.WaitPop(quit);
		cout << s << endl;
		logstream << s << endl;
	}});
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
