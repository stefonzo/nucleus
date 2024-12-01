#pragma once

// constants for buffer argument and for the PSP screen
#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)

#define GU_LIST_SIZE 262144

namespace nucleus 
{
	class vertex 
	{
	public:
		unsigned int color;
		float x, y, z;
		vertex(float vx, float vy, float vz, unsigned int vcolor);
	};
	
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
	};

	void initGraphics(void *list);
	void initMatrices(void);
	void startFrame(void *list);
	void endFrame(void);
	void termGraphics(void);
}
