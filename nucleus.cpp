#include "nucleus.h"
#include "callbacks.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace nucleus
{
	// data types

	mesh::mesh(unsigned int n_vertices, unsigned int index_count)
	{
		n_mesh_vertices = n_vertices;
		vertices = (vertex*)memalign(16, sizeof(vertex) * n_vertices);
		vertex_indices = (unsigned short*)memalign(16, sizeof(unsigned short) * index_count);
		n_indices = index_count;
	}

	mesh::~mesh()
	{
		free(vertices);
		free(vertex_indices);
	}

	void mesh::insertVertex(vertex v, unsigned int vn) 
	{
		if (vn >= n_mesh_vertices) { // check to make sure you don't insert a vertex that is out of array bounds!
			return;
		} else {
			vertices[vn] = v;
		}
	}

	void mesh::insertIndex(unsigned short val, unsigned int vertex_index) 
	{
		if (vertex_index >= n_indices) {
			return;
		} else {
			vertex_indices[vertex_index] = val;
		}
	}

	void mesh::renderMesh(void)
	{
		sceGumDrawArray(GU_TRIANGLES, PSP_PRIMITIVE_VERTICES, n_indices, vertex_indices, vertices);
	}
	
	template <typename T>
	void quad<T>::changePosition(ScePspFVector3 * pos)
	{
		quad_pos.x = pos->x, quad_pos.y = pos->y, quad_pos.z = pos->z;
	}

	texture_quad::texture_quad(float twidth, float theight, ScePspFVector3 *pos, unsigned int color)
	{
		width = twidth, height = theight;
		quad_pos.x = pos->x, quad_pos.y = pos->y;
		quad_pos.z = 0.0f;
		
		tex_vertex t0 = {0.0f, 0.0f, color, 0.0f, -height, 0.0f};
		tex_vertex t1 = {1.0f, 0.0f, color, width, -height, 0.0f};
		tex_vertex t2 = {1.0f, 1.0f, color, width, 0.0f, 0.0f};
		tex_vertex t3 = {0.0f, 1.0f, color, 0.0f, 0.0f, 0.0f};

		vertices[0] = t0, vertices[1] = t1, vertices[2] = t2, vertices[3] = t3;
		vertex_indices[0] = 0, vertex_indices[1] = 1, vertex_indices[2] = 2, vertex_indices[3] = 0, vertex_indices[4] = 2, vertex_indices[5] = 3;
		sceKernelDcacheWritebackInvalidateAll();  
	}

	texture_quad::~texture_quad()
	{
		
	}

	void texture_quad::render(void)
	{
		// transform quad
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		sceGumTranslate(&quad_pos);
		// render quad
		sceGumDrawArray(GU_TRIANGLES, PSP_TEXTURE_VERTICES, N_QUAD_INDICES, vertex_indices, vertices);
	}

	lit_texture_quad::lit_texture_quad(float twidth, float theight, ScePspFVector3 *pos, unsigned int color)
	{
		width = twidth, height = theight;
		quad_pos.x = pos->x, quad_pos.y = pos->y;
		quad_pos.z = 0.0f;

		tcnp_vertex v0 = {0.0f, 0.0f, color, 0.0f, 0.0f, 1.0f, 0.0f, -height, 0.0f};
		tcnp_vertex v1 = {1.0f, 0.0f, color, 0.0f, 0.0f, 1.0f, width, -height, 0.0f};
		tcnp_vertex v2 = {1.0f, 1.0f, color, 0.0f, 0.0f, 1.0f, width, 0.0f, 0.0f};
		tcnp_vertex v3 = {0.0f, 1.0f, color, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f};

		vertices[0] = v0, vertices[1] = v1, vertices[2] = v2, vertices[3] = v3;
		vertex_indices[0] = 0, vertex_indices[1] = 1, vertex_indices[2] = 2, vertex_indices[3] = 0, vertex_indices[4] = 2, vertex_indices[5] = 3;
		sceKernelDcacheWritebackInvalidateAll();  
	}

	lit_texture_quad::~lit_texture_quad() {}

	void lit_texture_quad::render(void)
	{
		// transform quad
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		sceGumTranslate(&quad_pos);
		// render quad
		sceGumDrawArray(GU_TRIANGLES, PSP_TEXTURE_NORMAL_VERTICES, N_QUAD_INDICES, vertex_indices, vertices);
	}

	void texture::loadTexture(const char *filename, const int vram) // use GU_TRUE for vram parameter
	{
		stbi_set_flip_vertically_on_load(GU_FALSE);
		unsigned char *data = stbi_load(filename, &width, &height, &nr_channels, STBI_rgb_alpha);
		pspDebugScreenSetXY(0, 0);
		if (!data) {
			texture_data = nullptr;
			writeToLog("Unable to load texture!");
			return;
		} 

		pixel_width = pow2(width);
		pixel_height = pow2(height);

		void *data_buffer = (unsigned int *)memalign(16, pixel_width * pixel_height * 4);

		copy_texture_data(data_buffer, data);

		stbi_image_free(data);
		
		unsigned int *swizzled_pixels = nullptr;
		if (vram) 
		{
			swizzled_pixels = (unsigned int *)getStaticVramTexture(pixel_width, pixel_height, GU_PSM_8888);
			writeToLog("Texture loaded into ram.\n");
		} else
		{
			swizzled_pixels = (unsigned int *)memalign(16, pixel_height * pixel_width * 4);
		}

		swizzle_fast((u8*)swizzled_pixels, (const u8*) data_buffer, pixel_width * 4, pixel_height);

		free(data_buffer);
		texture_data = swizzled_pixels;
		char buff[256];
		sprintf(buff, "Texture allocated at: %p", texture_data);
		writeToLog(buff);
		sceKernelDcacheWritebackInvalidateAll();
	}

	texture::texture(const char *filename, const int vram)
	{
		loadTexture(filename, vram);
	}

	texture::~texture()
	{

	}

	void texture::bindTexture(void)
	{
		if (texture_data == nullptr) {
			//writeToLog("Unable to bind texture!");
			return;
		} else {
			//writeToLog("Texture bound!");
		}
			
		sceGuTexMode(GU_PSM_8888, 0, 0, 1);
    	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
		sceGuTexImage(0, pixel_width, pixel_height, pixel_width, texture_data);
	}

	void texture::swizzle_fast(u8 *out, const u8 *in, const unsigned int width, const unsigned int height) // from Iridescentrose
	{
		unsigned int blockx, blocky;
    	unsigned int j;

    	unsigned int width_blocks = (width / 16);
    	unsigned int height_blocks = (height / 8);

    	unsigned int src_pitch = (width - 16) / 4;
    	unsigned int src_row = width * 8;

    	const u8 *ysrc = in;
    	u32 *dst = (u32 *)out;

    	for (blocky = 0; blocky < height_blocks; ++blocky) {
        	const u8 *xsrc = ysrc;
        	for (blockx = 0; blockx < width_blocks; ++blockx) {
            	const u32 *src = (u32 *)xsrc;
            	for (j = 0; j < 8; ++j) {
                	*(dst++) = *(src++);
                	*(dst++) = *(src++);
                	*(dst++) = *(src++);
                	*(dst++) = *(src++);
                	src += src_pitch;
            	}
            	xsrc += 16;
        	}
        	ysrc += src_row;
    	}
	}

	unsigned int texture::pow2(const unsigned int val)
	{
		unsigned int poweroftwo = 1;
    	while (poweroftwo < val) {
        	poweroftwo <<= 1;
    	}
    	return poweroftwo;
	}

	void texture::copy_texture_data(void *dest, const void *src)
	{
		for (int y = 0; y < height; y++) {
        	for (int x = 0; x < width; x++) {
        		((unsigned int*)dest)[x + y * pixel_width] = ((unsigned int *)src)[x + y * width];
       		}
    	}
	}

	 texture_manager::texture_manager() {}
	 texture_manager::~texture_manager() {}

	void texture_manager::addTexture(std::string filename)
	{
		texture temp_texture = texture(filename.c_str(), GU_TRUE);
		if (temp_texture.getTextureData() == nullptr) { return; }
		textures.insert({filename, temp_texture});
	}

	void texture_manager::removeTexture(std::string filename)
	{
		textures.erase(filename);
	}

	camera2D::camera2D(float x, float y) 
	{
		camera_pos.x = x;
		camera_pos.y = y;
		camera_pos.z = 0.0f;
		camera_target = camera_pos;
	}

	void camera2D::updateCameraTarget(float x, float y)
	{
		camera_target.x = x;
		camera_target.y = y;
	}

	const ScePspFVector3 camera2D::getCameraPosition(void)
	{
		return camera_pos;
	}

	void camera2D::smoothCameraUpdate(float dt)
	{
		float smoothing_factor = 30.0f;
		// linear interpolation
		camera_pos.x += (camera_target.x - camera_pos.x) * smoothing_factor * dt;
		camera_pos.y += (camera_target.y - camera_pos.y) * smoothing_factor * dt;
	}

	void camera2D::setCamera(void) 
	{
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();
		ScePspFVector3 translated_pos = {-camera_pos.x, -camera_pos.y, camera_pos.z};
		sceGumTranslate(&translated_pos);
	}

	// nucleus methods

	void writeToLog(const char *message)
	{
    	int fd = sceIoOpen(LOG_FILE, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    	if (fd >= 0) {
        	sceIoWrite(fd, message, strlen(message));
        	sceIoWrite(fd, "\n", 1); // Add a newline
        	sceIoClose(fd);
    	} else {
        	pspDebugScreenPrintf("Failed to open log file!\n");
    	}
	}

	void *getStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm)
	{
		static unsigned int offset = 0;
		unsigned int buffer_size;
		if (psm == GU_PSM_8888) {
			buffer_size = width * height * 4;
		} else if (psm == GU_PSM_4444) {
			buffer_size = width * height * 2;
		} else {
			return nullptr;
		}
		void *buffer = (void*)offset;
		offset += buffer_size;
		if (offset >= 2 * 1024 * 1024) { // don't want to exceed 2Mb vram range
			return nullptr;
		}
		return buffer;
	}

	void *getStaticVramTexture(unsigned int width, unsigned int height, unsigned int psm)
	{
		void *texture = getStaticVramBuffer(width, height, psm);
		return (void*)(((unsigned int)texture) + ((unsigned int)sceGeEdramGetAddr()));
	}

	void initGraphics(void *list)
	{
		// allocate memory in vram for draw, display, and zbuffers
		char buff[256];
		//void *draw_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		void *draw_buffer = getStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		// sprintf(buff, "Draw buffer allocated at: %p", draw_buffer);
		// writeToLog(buff);
		//void *disp_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		void *disp_buffer = getStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		// sprintf(buff, "Display buffer allocated at: %p", disp_buffer);
		// writeToLog(buff);
		//void *z_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_4444);
		void *z_buffer = getStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_4444);
		// sprintf(buff, "Z buffer allocated at: %p", z_buffer);
		// writeToLog(buff);
		// configure Gu
		sceGuInit();
		sceGuStart(GU_DIRECT, list);
		sceGuDrawBuffer(GU_PSM_8888, draw_buffer, PSP_BUF_WIDTH);
		sceGuDispBuffer(PSP_SCR_WIDTH, PSP_SCR_HEIGHT, disp_buffer, PSP_BUF_WIDTH);
		sceGuDepthBuffer(z_buffer, PSP_BUF_WIDTH);

		sceGuOffset(2048 - (PSP_SCR_WIDTH/2), 2048 - (PSP_SCR_HEIGHT/2)); // center the screen offset within the Gu coordinate space
		sceGuViewport(2048, 2048, PSP_SCR_WIDTH, PSP_SCR_HEIGHT); // define rendering viewport 
		sceGuDepthRange(65535, 0);

		sceGuScissor(0, 0, PSP_SCR_WIDTH, PSP_SCR_HEIGHT); // Restricts rendering to rectangle with size of PSP screen
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuDepthFunc(GU_GEQUAL);
		sceGuEnable(GU_DEPTH_TEST); // I don't think we're using the zbuffer for depth testing in our main program loop so this is redundant
		sceGuFrontFace(GU_CW); // render triangle vertices in a clockwise order
		sceGuShadeModel(GU_SMOOTH); // how pixels in our triangle will look? (Look at include)
		sceGuDisable(GU_CULL_FACE); // back facing triangles aren't rendered
		sceGuEnable(GU_TEXTURE_2D); // so we can use 2D textures later
		sceGuEnable(GU_CLIP_PLANES); // want view clipping
		sceGuFinish();               // done configuring Gu, tell the Gu to execute list instructions we sent it
		sceGuSync(0, 0);

		sceDisplayWaitVblankStart(); // helps prevent screen tearing (don't display to the screen until after vblank!)
		sceGuDisplay(GU_TRUE); // need this or I'm pretty sure nothing will dipslay to the screen...
	}

	void initMatrices(void)
	{
		// projection matrix (view/camera coords -> NDC)
		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumOrtho(0.0f, PSP_SCR_WIDTH, PSP_SCR_HEIGHT, 0.0f, -1.0f, 1.0); // origin is at upper left corner

		// view matrix (world coords -> view/camera coords)
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();

		// model matrix (model coords -> world coords)
		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
	}

	void initLighting(void *list)
	{
		sceGuStart(GU_DIRECT, list);
		sceGuEnable(GU_LIGHTING);

    	// Setup light 0 as a directional light
    	sceGuEnable(GU_LIGHT0);
    	ScePspFVector3 light_direction = {0.0f, 0.0f, 1.0f}; // Light coming from the viewer towards the screen
    	sceGuLight(0, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &light_direction);

    	// Set light colors (RGBA)
    	sceGuLightColor(0, GU_DIFFUSE, GU_COLOR(1.0f, 1.0f, 1.0f, 1.0f)); // White diffuse light
    	sceGuLightColor(0, GU_SPECULAR, GU_COLOR(0.1f, 0.1f, 0.1f, 1.0f)); // Less intense specular light

    	// Set ambient light globally (RGBA)
    	sceGuAmbient(GU_COLOR(0.4f, 0.4f, 0.4f, 1.0f)); // Dim ambient light

		sceGuMaterial(GU_AMBIENT_AND_DIFFUSE, GU_COLOR(0.0f, 1.0f, 0.0f, 1.0f)); // Pinkish material

		sceGuFinish();
		sceGuSync(0, 0);
		sceDisplayWaitVblankStart();
	}

	void readController(SceCtrlData ctrl_data, camera2D *game_camera)
	{
		sceCtrlReadBufferPositive(&ctrl_data, 1);

		if (ctrl_data.Buttons & PSP_CTRL_UP) 
		{
			game_camera->updateCameraTarget(game_camera->getCameraPosition().x, game_camera->getCameraPosition().y - CAMERA_CLAMPING);
		}
		if (ctrl_data.Buttons & PSP_CTRL_DOWN) 
		{
			game_camera->updateCameraTarget(game_camera->getCameraPosition().x, game_camera->getCameraPosition().y + CAMERA_CLAMPING);
		}
		if (ctrl_data.Buttons & PSP_CTRL_LEFT) 
		{
			game_camera->updateCameraTarget(game_camera->getCameraPosition().x - CAMERA_CLAMPING, game_camera->getCameraPosition().y);
		}
		if (ctrl_data.Buttons & PSP_CTRL_RIGHT) 
		{
			game_camera->updateCameraTarget(game_camera->getCameraPosition().x + CAMERA_CLAMPING, game_camera->getCameraPosition().y);
		}
	}

	void startFrame(void *list)
	{
		sceGuStart(GU_DIRECT, list);
	}

	void endFrame(void)
	{
		sceGuFinish();
		sceGuSync(0, 0);
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
	}

	void termGraphics(void)
	{
		sceGuTerm();
	}

	float calculateDeltaTime(u64 &last_time) // returns delta time in seconds
	{
		u64 current_time;
		sceRtcGetCurrentTick(&current_time);
		const u64 ticks_per_second = sceRtcGetTickResolution();
		float dt = (current_time - last_time) / (float)ticks_per_second;
		last_time = current_time;
		return dt;
	}	

	void setRenderMode(render_mode mode, void *list)
	{
		sceGuStart(GU_DIRECT, list);
		if (mode == render_mode::NUCLEUS_PRIMITIVES) {
			sceGuDisable(GU_TEXTURE_2D);
		} else if (mode == render_mode::NUCLEUS_TEXTURE2D) {
			sceGuEnable(GU_TEXTURE_2D);
		} else if (mode == render_mode::NUCLEUS_LIGHTING2D) {
			sceGuEnable(GU_TEXTURE_2D);
			sceGuEnable(GU_LIGHTING);
		}
		sceGuFinish();
		sceGuSync(0,0);
		sceDisplayWaitVblankStart();
	}

	namespace primitive
	{
		rectangle::rectangle(float width, float height, unsigned int color, ScePspFVector3 position) 
		{
			w = width, h = height;
			rectangle_color = color;
			rectangle_pos = position;
			rectangle_pos.z = 0.0f;

			// initialize mesh data
			vertex v1 = {color, 0.0f, -height, 0.0f};
			vertex v2 = {color, width, -height, 0.0f};
			vertex v3 = {color, width, 0.0f, 0.0f};
			vertex v4 = {color, 0.0f, 0.0f, 0.0f};
			rectangle_mesh.insertVertex(v1, 0);
			rectangle_mesh.insertVertex(v2, 1);
			rectangle_mesh.insertVertex(v3, 2);
			rectangle_mesh.insertVertex(v4, 3);

			rectangle_mesh.insertIndex(0, 0);
			rectangle_mesh.insertIndex(1, 1);
			rectangle_mesh.insertIndex(2, 2);

			rectangle_mesh.insertIndex(0, 3);
			rectangle_mesh.insertIndex(2, 4);
			rectangle_mesh.insertIndex(3, 5);
			sceKernelDcacheWritebackInvalidateAll();
		}

		void rectangle::changePosition(ScePspFVector3 *position)
		{rectangle_pos.x = position->x, rectangle_pos.y = position->y, rectangle_pos.z = position->z;}

		void rectangle::render(void) 
		{
			// transform model
			sceGumMatrixMode(GU_MODEL);
			sceGumLoadIdentity();
			sceGumTranslate(&rectangle_pos);
			// render rectangle
			rectangle_mesh.renderMesh();
		}
	}
}