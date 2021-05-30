#include "squareplane.h"
#include<la.h>
#include <iostream>

SquarePlane::SquarePlane(OpenGLContext *mp_context) : Drawable(mp_context)
{}

void SquarePlane::create()
{

    std::vector<glm::vec4> pos {glm::vec4(-2, -2, 0, 1),
                                glm::vec4(2, -2, 0, 1),
                                glm::vec4(2, 2, 0, 1),
                                glm::vec4(-2, 2, 0, 1)};

    std::vector<glm::vec4> nor {glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0),
                                glm::vec4(0, 0, 1, 0)};

    std::vector<glm::vec4> col {glm::vec4(1, 0, 0, 1),
                                 glm::vec4(0, 1, 0, 1),
                                 glm::vec4(0, 0, 1, 1),
                                 glm::vec4(1, 1, 0, 1)};

    std::vector<GLuint> idx {0, 1, 2, 0, 2, 3};

    count = 6; // TODO: Set "count" to the number of indices in your index VBO

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}
