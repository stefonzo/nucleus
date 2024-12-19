#pragma once

#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <pspdebug.h>

#include <malloc.h>


// constants for buffer argument and for the PSP screen
#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)
#define N_QUAD_VERTICES (4)
#define N_QUAD_INDICES (6) // 3 triangles for a quad

#define GU_LIST_SIZE 262144

namespace nucleus 
{
	enum class render_mode
	{
		NUCLEUS_PRIMITIVES, NUCLEUS_TEXTURE2D
	};

	struct vertex 
	{
		unsigned int color;
		float x, y, z;
	} __attribute__((aligned(16)));

	struct tex_vertex
	{
		float u, v;
		unsigned int color;
		float x, y, z;
	} __attribute__((aligned(16)));
	
	class mesh 
	{
	public:
		mesh(unsigned int n_vertices, unsigned int index_count);
		~mesh();
		void insert_vertex(vertex v, unsigned int vn);
		void insert_index(unsigned short val, unsigned int vertex_index);
		void render_mesh(void);
	private:
		vertex *vertices;
		unsigned short *vertex_indices; // 16 bit
		unsigned int n_mesh_vertices, n_indices;		// 32 bit
	} __attribute__((aligned(16)));

	class texture_quad
	{
	public:
		texture_quad(float twidth, float theight, ScePspFVector3 pos, unsigned int color);
		~texture_quad();
		void render(void);
		void change_position(ScePspFVector3 pos) { quad_pos = pos;}
	private:
		tex_vertex __attribute__((aligned(16)))vertices[N_QUAD_VERTICES];
		unsigned short __attribute__((aligned(16)))vertex_indices[N_QUAD_INDICES];
		float width, height; 
		ScePspFVector3 quad_pos;
	} __attribute__((aligned(16)));

	class texture
	{
	public:
		void load_texture(const char *filename, const int vram); // use GU_TRUE for vram parameter
		texture(const char *filename, const int vram);
		~texture();
		void bind_texture(void);
		int get_width(void) {return width;}
		int get_height(void) {return height;}
		int get_pixel_width(void) {return pixel_width;}
		int get_pixel_height(void) {return pixel_height;}
		void *get_texture_data(void) {return texture_data;}
	private:
		void *texture_data;
		int width, height, pixel_width, pixel_height, nr_channels;
		unsigned int pow2(const unsigned int val);
		void swizzle_fast(u8 *out, const u8 *in, const unsigned int width, const unsigned int height);
		void copy_texture_data(void *dest, const void *src);
	};

	class camera2D 
	{
	public:
		camera2D(float x, float y);
		void updateCameraTarget(float x, float y);
		const ScePspFVector3 getCameraPosition(void);
		void smoothCameraUpdate(float dt);
		void setCamera(void);
	private:
		ScePspFVector3 camera_pos;
		ScePspFVector3 camera_target;
	};

	void initGraphics(void *list);
	void initMatrices(void);
	void startFrame(void *list);
	void endFrame(void);
	void termGraphics(void);
	void setRenderMode(render_mode mode, void *list);
	float calculateDeltaTime(u64 &last_time);

	namespace primitive
	{
		class rectangle
		{
			public:
				rectangle(float width, float height, unsigned int color, ScePspFVector3 position);
				void changePosition(ScePspFVector3 position);
				void setWidth(float width);
				void setHeight(float height);
				float getWidth(void);
				float getHeight(void);
				void render(void);
			private:
				mesh rectangle_mesh = mesh(4, 6);
				ScePspFVector3 rectangle_pos;
				float w, h;
				unsigned int rectangle_color;
		}__attribute__((aligned(16)));
	}
}