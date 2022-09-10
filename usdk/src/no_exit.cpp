extern "C" void hard_assertion_failure() { while (true); }
extern "C" void panic() { while (true); }
extern "C" void exit(int) { while (true); }
extern "C" void atexit(void(*)()) { while (true); }
