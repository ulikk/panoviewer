#include <GL/glew.h>
#include "glutil.h"
#include <string>
#include <iostream>
#include <sstream>

namespace GLUTIL
{

   bool checkGLError(const std::string &msg)
   {
       std::ostringstream os;

       GLenum err = glGetError();
       if (err == GL_NO_ERROR) return false;

       os << "OPENGL ERROR [" << err << "] in " << msg << std::endl;

      switch(err)
      {
         case GL_INVALID_ENUM :
            os << "GLenum argument out of range"; break;
         case GL_INVALID_VALUE :
            os << "Numeric argument out of range"; break;
         case GL_INVALID_OPERATION :
            os << "Operation illegal in current state"; break;
         case GL_STACK_OVERFLOW :
            os << "Command would cause a stack overflow"; break;
         case GL_STACK_UNDERFLOW :
            os << "Command would cause a stack underflow"; break;
         case GL_OUT_OF_MEMORY :
            os << "Not enough memory left to execute command"; break;
         default:
            os << "Uh Oh, i don't know the error code."; break;
      }
      std::cout << os.str() << std::endl;
      return true;
   }

}
