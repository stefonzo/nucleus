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
	pspDebugScreenInit();

	// initialize data for drawing mesh to screen
	float square_size = 50.0f;
	nucleus::vertex v1 = nucleus::vertex(-square_size, -square_size, 0.0f, 0xFFFF0000);
	nucleus::vertex v2 = nucleus::vertex(-square_size, square_size, 0.0f, 0xFF00FF00);
	nucleus::vertex v3 = nucleus::vertex(square_size, square_size, 0.0f, 0xFF0000FF);
	nucleus::vertex v4 = nucleus::vertex(square_size, -square_size, 0.0f, 0xFF0000F0);

	nucleus::mesh square = nucleus::mesh(4, 6);

	square.insert_vertex(v1, 0);
	square.insert_vertex(v2, 1);
	square.insert_vertex(v3, 2);
	square.insert_vertex(v4, 3);

	square.insert_index(0, 0);
	square.insert_index(2, 1);
	square.insert_index(1, 2);

	square.insert_index(0, 3);
	square.insert_index(3, 4);
	square.insert_index(2, 5);

	ScePspFVector3 pos;
	pos.x = 10.0f, pos.y = 10.0f;
	nucleus::primitive::rectangle rect = nucleus::primitive::rectangle(240.0f, 136.0f, 0xFF00FF00, pos);

	ScePspFVector3 pos2;
	pos2.x = PSP_SCR_WIDTH / 2, pos2.y = PSP_SCR_HEIGHT / 2;
	nucleus::primitive::rectangle rect2 = nucleus::primitive::rectangle(100.0f, 100.0f, 0xFFFF0000, pos2);

	// testing out a 2D camera
	nucleus::camera2D camera = nucleus::camera2D(0.0f, 0.0f); // have camera looking at the center of the screen

	// variable to store controller info
	SceCtrlData ctrlData;

	u64 lastTime;
	sceRtcGetCurrentTick(&lastTime);

	while (running)
	{
		float dt = nucleus::calculateDeltaTime(lastTime);
		pspDebugScreenPrintf("Delta time: %f\n", dt);
		nucleus::startFrame(gu_list);
		sceGuDisable(GU_DEPTH_TEST);

		// blending
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuEnable(GU_BLEND);

		// clear background to white
		sceGuClearColor(0xFFFFFFFF);
		sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);

		// testing out controller input
		sceCtrlReadBufferPositive(&ctrlData, 1);

		if (ctrlData.Buttons & PSP_CTRL_UP) 
		{
			camera.updateCameraTarget(camera.getCameraPosition().x, camera.getCameraPosition().y - 10.0f);
		}
		if (ctrlData.Buttons & PSP_CTRL_DOWN) 
		{
			camera.updateCameraTarget(camera.getCameraPosition().x, camera.getCameraPosition().y + 10.0f);
		}
		if (ctrlData.Buttons & PSP_CTRL_LEFT) 
		{
			camera.updateCameraTarget(camera.getCameraPosition().x - 10.0f, camera.getCameraPosition().y);
		}
		if (ctrlData.Buttons & PSP_CTRL_RIGHT) 
		{
			camera.updateCameraTarget(camera.getCameraPosition().x + 10.0f, camera.getCameraPosition().y);
		}

		camera.smoothCameraUpdate(dt);
		camera.setCamera();

		// do transform stuff for mesh rendering
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity(); // no transforms for rendering this mesh (theoretically one vertex of the triangle should be located at the center of the screen...)
		ScePspFVector3 square_position = {240.0f, 136.0f, 0.0f};
		sceGumTranslate(&square_position);

		// render mesh
		//square.render_mesh();

		// render new rectangle
		rect.render();
		rect2.render();

		nucleus::endFrame();
	}
	nucleus::termGraphics();
	sceKernelExitGame();
	return 0;
}