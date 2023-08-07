#include <android/log.h>
#include <"WpHoook.h">
#include <"LOG.h>
#include <"AndroidTP.h">
using namespace std;

double justhok,opt,hook0addr;
long int address,atp;

void hook_read_wpbit(long int addr)
{
    address = hook_read(addr);
    long int var = 0;
}

void hook_write_wpbit(long int addr)
{
    address = hook_write(addr);
    long int var = 0;
}

void hook0_wpbit(long int addr)
{
    thook.GetHookPT(addr);
}
