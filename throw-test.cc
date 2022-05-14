#include <iostream>
#include <hamlib/rigclass.h>
#include <hamlib/ampclass.h>

int main (void)
{
#ifdef __MINGW32__
  puts ("Hello from MinGW's 'main()'");
#else
  puts ("Hello from MSVC's 'main()'");
#endif

#if (__cplusplus >= 201103L)  // since in >= C++11, the constructor gets deleted
  Amplifier *a = nullptr;

  try {
    a = new Amplifier (100);  // > AMP_BACKEND_MAX
  }
  catch (const RigException &Ex) {
    std::cout << "Exception: " << Ex.message << std::endl;
  }

  if (a)
     delete a;

#else
  try {
    Amplifier a(100);  // > AMP_BACKEND_MAX
  }
  catch (const RigException &Ex) {
    std::cout << "Exception: " << Ex.message << std::endl;
  }
#endif
  return (0);
}

/*
 A MinGW compiled version:

Hello from MinGW's 'main()'
amp_init called
amp_check_backend: unsupported backend 1 for model 100
terminate called after throwing an instance of 'RigException'

ntdll!NtTerminateProcess+0xc
ntdll!RtlExitUserProcess+0xbd
KERNEL32!ExitProcessImplementation+0x12
msvcrt!__crtExitProcess+0x17
msvcrt!doexit+0x135
msvcrt!_exit+0x11
msvcrt!abort+0xe8
libstdc___6!ZN9__gnu_cxx27__verbose_terminate_handlerEv+0x1a6
throw_test!std::operator<<<std::char_traits<char> >(class std::basic_ostream<char,std::char_traits<char> > * _Ostr = <Value unavailable error>, char * _Val = <Value unavailable error>)+0x256
KERNEL32!BaseThreadInitThunk+0x19
ntdll!__RtlUserThreadStart+0x2f
ntdll!_RtlUserThreadStart+0x1b

*/
