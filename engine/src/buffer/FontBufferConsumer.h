#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
using namespace std;
using namespace glm;

class FontBufferConsumer {
    const BufferMargins margins;
    const BufferStorage* buffer;
    unsigned long vertex_total;
    unsigned long index_total;
public:
    FontBufferConsumer(BufferStorage* buffer) {
        this->buffer = buffer;
        margins = buffer->RequestStorage(BufferSettings::Margins::FONT_VERTEX_PART, BufferSettings::Margins::FONT_INDEX_PART);
        vertex_total = margins.start_vertex;
        index_total = margins.start_index;
    }
    ~FontBufferConsumer() {
    }
    void Reset() {
        vertex_total = 0;
        index_total = 0;
    };
}