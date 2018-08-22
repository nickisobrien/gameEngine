#pragma once

#include "shader.h"
#include "block.h"
#include "FastNoise.h"

#define CHUNK_Z 16
#define CHUNK_X 16
#define CHUNK_Y 256

enum ChunkState
{
	GENERATE,
	UPDATE,
	RENDER,
	UNLOAD
};

class Chunk
{
public:
	Chunk(int xoff = 0, int zoff = 0);
	~Chunk(void);
	void update(void);
	void render(Shader shader);
	void renderWater(Shader shader);
	void faceRendering(void);
	void buildVAO(void);
	void addFace(int face, int x, int y, int z, int val);
	void addTransparentFace(int face, int x, int y, int z, int val);
	void cleanVAO(void);
	
	// neighbors
	void setXMinus(Chunk *chunk);
	void setXPlus(Chunk *chunk);
	void setZMinus(Chunk *chunk);
	void setZPlus(Chunk *chunk);
	Chunk *getXMinus();
	Chunk *getXPlus();
	Chunk *getZMinus();
	Chunk *getZPlus();

	void setTerrain(FastNoise terrainNoise, FastNoise temperatureNoise, FastNoise humidityNoise);
	void addExtras(FastNoise terrainNoise, FastNoise temperatureNoise, FastNoise humidityNoise);
	int	getWorld(int x, int y, int z);
	bool neighborsSet = false;
	Block *getBlock(int x, int y, int z);
	inline int getXOff() { return xoff; }
	inline int getZOff() { return zoff; }
private:
	friend class Player;
	int xoff;
	int zoff;
	Block ***blocks;
	glm::mat4 offsetMatrix;
	unsigned int VAO;
	unsigned int VBO_VERT;
	unsigned int VBO_UV;

	int pointSize;
	int transparentPointSize;

	unsigned int transparentVAO;
	unsigned int transparentVBO_VERT;
	unsigned int transparentVBO_UV;


	vector<glm::vec3> points;
	vector<glm::vec2> uvs;
	vector<glm::vec3> transparentPoints;
	vector<glm::vec2> transparentUvs;
	ChunkState state = GENERATE;
	Chunk *xMinus = NULL;
	Chunk *xPlus = NULL;
	Chunk *zMinus = NULL;
	Chunk *zPlus = NULL;
};