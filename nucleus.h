#pragma once

#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

#include <string>
#include <unordered_map>
#include <cstdio>
#include <malloc.h>

#define LOG_FILE "ms0:/log.txt"

// constants for buffer argument and for the PSP screen
#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)
#define N_QUAD_VERTICES (4)
#define N_QUAD_INDICES (6) // 3 triangles for a quad

// rendering function argument macros
#define PSP_PRIMITIVE_VERTICES (GU_INDEX_16BIT | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D)
#define PSP_TEXTURE_VERTICES (GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D)
#define PSP_TEXTURE_NORMAL_VERTICES (GU_INDEX_16BIT | GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D)

#define CAMERA_CLAMPING 10.0f

#define GU_LIST_SIZE 262144

namespace nucleus 
{
	enum class render_mode
	{
		NUCLEUS_PRIMITIVES, NUCLEUS_TEXTURE2D, NUCLEUS_LIGHTING2D
	};

	struct vertex 
	{
		unsigned int color;
		float x, y, z;
	};

	struct tex_vertex
	{
		float u, v;
		unsigned int color;
		float x, y, z;
	};

	struct tcnp_vertex // 'texture, color, normal, position'
	{
		float u, v;
		unsigned int color;
		float nx, ny, nz;
		float x, y, z;
	};
	
	class mesh 
	{
	public:
		mesh(unsigned int n_vertices, unsigned int index_count);
		~mesh();
		void insertVertex(vertex v, unsigned int vn);
		void insertIndex(unsigned short val, unsigned int vertex_index);
		void renderMesh(void);
	private:
		vertex *vertices;
		unsigned short *vertex_indices; // 16 bit
		unsigned int n_mesh_vertices, n_indices;		// 32 bit
	} __attribute__((aligned(16)));

	template<typename T>
	class quad
	{
	public:
		virtual void render(void) = 0;
		virtual ~quad() = default;
		void changePosition(ScePspFVector3 *position);
		T __attribute__((aligned(16)))vertices[N_QUAD_VERTICES];
	protected:
		unsigned short __attribute__((aligned(16)))vertex_indices[N_QUAD_INDICES];
		float width, height;
		ScePspFVector3 quad_pos;
	};

	class texture_quad : public quad<tex_vertex>
	{
	public:
		texture_quad(float twidth, float theight, ScePspFVector3 *pos, unsigned int color);
		~texture_quad();
		void render(void) override;
	} __attribute__((aligned(16)));

	class lit_texture_quad : public quad<tcnp_vertex>
	{
	public:
		lit_texture_quad(float twidth, float theight, ScePspFVector3 *pos, unsigned int color);
		~lit_texture_quad();
		void render(void) override;
	} __attribute__((aligned(16)));

	class texture
	{
	public:
		void loadTexture(const char *filename, const int vram); // use GU_TRUE for vram parameter
		texture(const char *filename, const int vram);
		~texture();
		void bindTexture(void);
		int getWidth(void) {return width;}
		int getHeight(void) {return height;}
		int getPixelWidth(void) {return pixel_width;}
		int getPixelHeight(void) {return pixel_height;}
		void *getTextureData(void) {return texture_data;}
		void setTextureData(void* data) {texture_data = data;} // I might not need this...
	private:
		void *texture_data;
		int width, height, pixel_width, pixel_height, nr_channels;
		unsigned int pow2(const unsigned int val);
		void swizzle_fast(u8 *out, const u8 *in, const unsigned int width, const unsigned int height);
		void copy_texture_data(void *dest, const void *src);
	};

	class texture_manager
	{
	public:
		texture_manager();
		~texture_manager();
		void addTexture(std::string filename);
		void removeTexture(std::string filename);
		std::unordered_map<std::string, texture> textures;
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

	// nucleus (game engine) methods
	void writeToLog(const char *message);
	void *getStaticVramBuffer(unsigned int width, unsigned int height, unsigned int psm);
	void *getStaticVramTexture(unsigned int width, unsigned int height, unsigned int psm);
	void initGraphics(void *list);
	void initMatrices(void);
	void initLighting(void *list);
	void readController(SceCtrlData ctrl_data, camera2D *game_camera);
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
				void changePosition(ScePspFVector3 *position);
				void setWidth(float width) {w = width;}
				void setHeight(float height) {h = height;}
				float getWidth(void) {return w;}
				float getHeight(void) {return h;}
				void render(void);
			private:
				mesh rectangle_mesh = mesh(4, 6);
				ScePspFVector3 rectangle_pos;
				float w, h;
				unsigned int rectangle_color;
		}__attribute__((aligned(16)));
	}
}