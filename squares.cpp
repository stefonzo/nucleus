#include "nucleus.h"
#include "callbacks.h"

#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <pspdebug.h>

#define printf pspDebugScreenPrintf

// PSP Module Info (necessary to create EBOOT.PBP)
PSP_MODULE_INFO("Squares", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main() 
{
	// initialize data
    bool running = true; // used in app loop
    static unsigned int __attribute__((aligned(16))) gu_list[GU_LIST_SIZE]; // used to send commands to the Gu

	setupCallbacks();
	nucleus::initGraphics(gu_list);
	nucleus::initMatrices();

	// testing out a 2D camera
	nucleus::camera2D camera = nucleus::camera2D(0.0f, 0.0f); // have camera looking at the center of the screen

	// variable to store controller info
	SceCtrlData ctrlData;

	// setting up data for textures
	nucleus::texture_manager demo_textures = nucleus::texture_manager();
	demo_textures.addTexture("spelunky_font.png");
	demo_textures.addTexture("circle.png");

	ScePspFVector3 font_pos = {PSP_SCR_WIDTH / 2, PSP_SCR_HEIGHT / 2, 0.0f};
	ScePspFVector3 circle_pos = {20.0f, 20.0f, 0.0f};
	ScePspFVector3 lit_circle_pos = {PSP_SCR_WIDTH / 2, PSP_SCR_HEIGHT / 2, 0.0f};

	nucleus::texture_quad font_quad = nucleus::texture_quad(demo_textures.textures.at("spelunky_font.png").get_pixel_width(), demo_textures.textures.at("spelunky_font.png").get_pixel_height(), font_pos, 0xFFFFFFFF);
	nucleus::texture_quad circle_quad = nucleus::texture_quad(50.0f, 50.0f, circle_pos, 0xFFFFFFFF);

	nucleus::lit_texture_quad lit_circle_quad = nucleus::lit_texture_quad(75.0f, 75.0f, lit_circle_pos);

	static nucleus::render_mode texture_test = nucleus::render_mode::NUCLEUS_TEXTURE2D;
	static nucleus::render_mode lighting_test = nucleus::render_mode::NUCLEUS_LIGHTING2D;

	nucleus::setRenderMode(texture_test, gu_list);	

	u64 lastTime;
	sceRtcGetCurrentTick(&lastTime);


	while (running)
	{
		nucleus::startFrame(gu_list);
		float dt = nucleus::calculateDeltaTime(lastTime);

		sceGuDisable(GU_DEPTH_TEST);
	
		// blending
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuEnable(GU_BLEND);

		// clear background to gray
		sceGuClearColor(0xFF888888);
		sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT);

		// testing out controller input
		// TODO put input code into a function
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

		// update and set camera
		camera.smoothCameraUpdate(dt);
		camera.setCamera();

		// render textured quads
		demo_textures.textures.at("spelunky_font.png").bind_texture();
		font_quad.render();

		demo_textures.textures.at("circle.png").bind_texture();
		circle_quad.render();

		// render lit quad
		// demo_textures.textures.at("circle.png").bind_texture();
		// lit_circle_quad.render();
		
		nucleus::endFrame();
	}
	nucleus::termGraphics();
	sceKernelExitGame();
	return 0;
}