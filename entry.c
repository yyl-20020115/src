#ifdef USE_LUA_ENTRY
#include <string.h>
extern int main_lua (int argc, char* argv[]);
extern int main_luac(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    int use_luac = 0;
    if (argc > 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-c") == 0) {
                use_luac = 1;
                break;
            }
        }
    }
    return use_luac 
        ? main_luac(argc,argv) 
        : main_lua (argc,argv)
        ;
}
#endif