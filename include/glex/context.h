#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <memory>
#include "program.h"

class Context {
private:
    std::unique_ptr<Program> program_;

public:
    static std::unique_ptr<Context> create();
    void render();

private:
    Context() {};
    bool init();
};


#endif // __CONTEXT_H__
