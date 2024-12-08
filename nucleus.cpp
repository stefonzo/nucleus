#include "nucleus.h"
#include "callbacks.h"

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

	void mesh::insert_vertex(vertex v, unsigned int vn) 
	{
		if (vn >= n_mesh_vertices) { // check to make sure you don't insert a vertex that is out of array bounds!
			return;
		} else {
			vertices[vn] = v;
		}
	}

	void mesh::insert_index(unsigned short val, unsigned int vertex_index) 
	{
		if (vertex_index >= n_indices) {
			return;
		} else {
			vertex_indices[vertex_index] = val;
		}
	}

	void mesh::render_mesh(void)
	{
		sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, n_indices, vertex_indices, vertices);
	}

	texture_quad::texture_quad(float twidth, float theight, unsigned int color)
	{
		width = twidth, height = theight;
		// tex_vertex t0 = tex_vertex(-0.5f * width, -0.5 * height, 0.0f, 0xFF000000, 0.0f, 0.0f);
		// tex_vertex t1 = tex_vertex(-0.5f * width, 0.5 * height, 0.0f, 0xFF000000, 1.0f, 0.0f);
		// tex_vertex t2 = tex_vertex(0.5f * width, -0.5f * height, 0.0f, 0xFF000000, 0.0f, 1.0f);
		// tex_vertex t3 = tex_vertex(0.5f * width, 0.5f * height, 0.0f, 0xFF000000, 1.0f, 1.0f);
		tex_vertex t0 = {0.0f, 0.0f, color, (-0.5f * width), (-0.5f * height), 0.0f};
		tex_vertex t1 = {1.0f, 0.0f, color, (-0.5f * width), (0.5f * height), 0.0f};
		tex_vertex t2 = {0.0f, 1.0f, color, (0.5f * width), (-0.5f * height), 0.0f};
		tex_vertex t3 = {1.0f, 1.0f, color, (0.5f * width), (0.5f * height), 0.0f};
		vertices[0] = t0, vertices[1] = t1, vertices[2] = t2, vertices[3] = t3;
		vertex_indices[0] = 0, vertex_indices[1] = 2, vertex_indices[2] = 1, vertex_indices[3] = 2, vertex_indices[4] = 3, vertex_indices[5] = 1;
		sceKernelDcacheWritebackInvalidateAll();  
	}

	texture_quad::~texture_quad()
	{

	}

	void texture::load_texture(const char *filename, const int vram) // use GU_TRUE for vram parameter
	{

	}

	texture::texture()
	{

	}

	texture::~texture()
	{

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

	// methods
	void initGraphics(void *list)
	{
		// allocate memory in vram for draw, display, and zbuffers
		void *draw_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		void *disp_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
		void *z_buffer = guGetStaticVramBuffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_4444);

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
		sceGuDisable(GU_DEPTH_TEST); // I don't think we're using the zbuffer for depth testing in our main program loop so this is redundant
		sceGuFrontFace(GU_CW); // render triangle vertices in a clockwise order
		sceGuShadeModel(GU_SMOOTH); // how pixels in our triangle will look? (Look at include)
		sceGuEnable(GU_CULL_FACE); // back facing triangles aren't rendered
		sceGuDisable(GU_TEXTURE_2D); // so we can use 2D textures later
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

	namespace primitive
	{
		rectangle::rectangle(float width, float height, unsigned int color, ScePspFVector3 position) 
		{
			w = width, h = height;
			rectangle_color = color;
			rectangle_pos = position;
			rectangle_pos.z = 0.0f;

			// initialize mesh data
			vertex v1 = {color, (-0.5f * width), (-0.5f * height), 0.0f};
			vertex v2 = {color, (-0.5f * width), (0.5f * height), 0.0f};
			vertex v3 = {color, (0.5f * width), (-0.5f * height), 0.0f};
			vertex v4 = {color, (0.5f * width), (0.5f * height), 0.0f};
			rectangle_mesh.insert_vertex(v1, 0);
			rectangle_mesh.insert_vertex(v2, 1);
			rectangle_mesh.insert_vertex(v3, 2);
			rectangle_mesh.insert_vertex(v4, 3);
			rectangle_mesh.insert_index(0, 0);
			rectangle_mesh.insert_index(2, 1);
			rectangle_mesh.insert_index(1, 2);
			rectangle_mesh.insert_index(2, 3);
			rectangle_mesh.insert_index(3, 4);
			rectangle_mesh.insert_index(1, 5);
			sceKernelDcacheWritebackInvalidateAll();
		}
		void rectangle::changePosition(ScePspFVector3 position)
		{rectangle_pos = position;}

		void rectangle::setWidth(float width) 
		{w = width;}

		void rectangle::setHeight(float height) 
		{h = height;}

		float rectangle::getWidth(void) 
		{return w;}

		float rectangle::getHeight(void) 
		{return h;}

		void rectangle::render(void) 
		{
			// transform model
			sceGumMatrixMode(GU_MODEL);
			sceGumLoadIdentity();
			sceGumTranslate(&rectangle_pos);
			// render rectangle
			rectangle_mesh.render_mesh();
		}
	}
}