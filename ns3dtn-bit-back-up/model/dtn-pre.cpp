#include "dtn-pre.h"

namespace ns3 {
    namespace ns3dtnbit {

#ifdef DEBUG
        /*
         * by default, get the function name called the logfunc which called this
         */
        string GetCallStack(int i = 2) {
            int nptrs;
            void *buffer[200];
            char **cstrings;
            char* return_str = new char[200];

            nptrs = backtrace(buffer, 200);
            sprintf(return_str, "backtrace() returned %d addresses\n", nptrs);
            /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
             *               would produce similar output to the following: */

            cstrings = backtrace_symbols(buffer, nptrs);
            if (cstrings == NULL || nptrs < 3) {
                perror("backtrace_symbols");
                exit(EXIT_FAILURE);
            }
            sprintf(return_str, "%s\n", cstrings[i]);
            free(cstrings);
            return return_str;
        }

        string FilePrint(string str) {
            std::stringstream ss;
            char* cs = new char[200];
            std::sprintf(cs, "file : %s, line : %d,", __FILE__, __LINE__);
            ss << "====== FilePrint ===== " << cs << "--->>" << str << endl;
            return ss.str();
        }

        string GetLogStr(string str) {
            std::stringstream ss;
            string caller = GetCallStack();
            ss << "==== Caller ====" << caller << "\n---->>" << str << endl;
            string Filep = FilePrint(ss.str());
            return Filep;
        }

#endif
    } /* ns3dtnbit */ 

} /* ns3  */ 
