#include "callbacks.h"

namespace nucleus
{
	int exit_callback(int arg1, int arg2, void* common)
	{
		sceKernelExitGame(); // stops game module 
		return 0;
	}

	int CallbackThread(SceSize args, void* argp)
	{
		int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL); // look up this method in api
		sceKernelRegisterExitCallback(cbid);
		sceKernelSleepThreadCB();
		return 0;
	}

	/* 
	* Sets up exit callback so you could quit this from a actual psp to the home menu
	*/

	int setupCallbacks(void) {
		int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
		if (thid >= 0) {
			sceKernelStartThread(thid, 0, 0);
		}
		return thid;
	}
}
