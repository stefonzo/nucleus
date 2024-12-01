#include "nucleus.h"
#include "callbacks.h"

#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>

// PSP Module Info (necessary to create EBOOT.PBP)
PSP_MODULE_INFO("Squares", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main() 
{
	// initialize data
    bool running = true; // used in app loop
    unsigned int __attribute__((aligned(16))) gu_list[GU_LIST_SIZE]; // used to send commands to the Gu

	setupCallbacks();
	nucleus::initGraphics((void*)gu_list);
	nucleus::initMatrices();

	// initialize data for drawing mesh to screen
	nucleus::vertex v1 = nucleus::vertex(-0.25f, -0.25f, 0.0f, 0xFFFF0000);
	nucleus::vertex v2 = nucleus::vertex(-0.25f, 0.25f, 0.0f, 0xFF00FF00);
	nucleus::vertex v3 = nucleus::vertex(0.25f, 0.25f, 0.0f, 0xFF0000FF);
	nucleus::vertex v4 = nucleus::vertex(0.25f, -0.25f, 0.0f, 0xFF0000F0);

	nucleus::mesh square = nucleus::mesh(4, 6);

	square.insert_vertex(v1, 0);
	square.insert_vertex(v2, 1);
	square.insert_vertex(v3, 2);
	square.insert_vertex(v4, 3);

	square.insert_index(0, 0);
	square.insert_index(1, 1);
	square.insert_index(2, 2);

	square.insert_index(2, 3);
	square.insert_index(3, 4);
	square.insert_index(0, 5);
	

	while (running)
	{
		nucleus::startFrame(gu_list);
		sceGuDisable(GU_DEPTH_TEST);

		// blending
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuEnable(GU_BLEND);

		// clear background to white
		sceGuClearColor(0xFFFFFFFF);
		sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);

		// do transform stuff for mesh rendering
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity(); // no transforms for rendering this mesh (theoretically one vertex of the triangle should be located at the center of the screen...)

		// render mesh
		square.render_mesh();

		nucleus::endFrame();
	}
	nucleus::termGraphics();
	sceKernelExitGame();
	return 0;
}
