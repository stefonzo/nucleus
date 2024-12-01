#include "nucleus.h"
#include "callbacks.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspge.h>
#include <pspdisplay.h>
#include <pspkernel.h>

#include <malloc.h>

namespace nucleus
{
	// data types

	vertex::vertex(float vx, float vy, float vz, unsigned int vcolor)
	{
		x = vx;
		y = vy;
		z = vz;
		color = vcolor;
	}

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
		sceGuEnable(GU_DEPTH_TEST); // I don't think we're using the zbuffer for depth testing in our main program loop so this is redundant
		sceGuFrontFace(GU_CW); // render triangle vertices in a clockwise order
		sceGuShadeModel(GU_SMOOTH); // how pixels in our triangle will look? (Look at include)
		sceGuEnable(GU_CULL_FACE); // back facing triangles aren't rendered
		//sceGuEnable(GU_TEXTURE_2D); // so we can use 2D textures later
		sceGuDisable(GU_TEXTURE_2D); // for testing purposes
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
		sceGumOrtho(-16.0f/9.0f, 16.0f/9.0f, -1.0f, 1.0f, -10.0f, 10.0f);

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
}
