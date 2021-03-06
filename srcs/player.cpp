#include <player.hpp>

void Player::processInput(GLFWwindow *window, float deltaTime)
{
	// DEBUGGERS
	// if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	// 	this->terr->updateChunk(glm::ivec2(this->getChunk()->getXOff(), this->getChunk()->getZOff()));
	// if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	// 	cout << this->getChunk()->neighborQueue.size() << endl;
	// if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	// 	cout << "Player position: (" << this->getPosition().x << ", " << this->getPosition().y << ", " << this->getPosition().z << ") " << endl;
	// if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
	// {
	// 	glm::vec3 current = this->getPosition();
	// 	int x = (int)floor(current.x) % CHUNK_X;
	// 	int y = floor(current.y);
	// 	int z = (int)floor(current.z) % CHUNK_Z;
	// 	if (x < 0)
	// 		x = CHUNK_X + x;
	// 	if (z < 0)
	// 		z = CHUNK_Z + z;

	// 	cout << "Block:	(" << x << ", " << y << ", " << z << ")" << endl;
	// }
	// if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	// 	this->currentBlockPlace = Blocktype::LIGHT_BLOCK;
	// if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	// 	this->currentBlockPlace = Blocktype::SNOW_BLOCK;

	// NON DEBUGGERS
	// Escape = exit window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Movement
	glm::vec3 savePos = this->getPosition();
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		this->camera->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		this->camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		this->camera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		this->camera->ProcessKeyboard(RIGHT, deltaTime);
	// collision checks/allows running up 1 block
	glm::vec3 newPos = this->getPosition();
	int x = (int)floor(newPos.x) % CHUNK_X;
	int y = floor(newPos.y) - 2;
	int z = (int)floor(newPos.z) % CHUNK_Z;
	if (x < 0)
		x = CHUNK_X + x;
	if (z < 0)
		z = CHUNK_Z + z;
	Chunk *c;
	if ((c = this->getChunk()) != NULL) // prob don't need the if statement now that getChunk will make chunk if needed
	{
		Block *b = c->getBlock(x,y,z);
		if (b != NULL && b->isActive())
		{
			b = c->getBlock(x, y+1, z);
			if (b != NULL && !b->isActive())
				this->setPosition(glm::vec3(newPos.x, newPos.y+1, newPos.z));
			else
				this->setPosition(savePos); // need to change to only reverting x/y/z, not necessarily all of them
		}
	}

	//Space for jumping
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		this->jump();
}

Chunk *Player::getChunk()
{
	int cx = this->camera->Position.x >= 0.0f ? this->camera->Position.x / CHUNK_X : ceil(this->camera->Position.x) / CHUNK_X - 1.0f;
	int cz = this->camera->Position.z >= 0.0f ? this->camera->Position.z / CHUNK_Z : ceil(this->camera->Position.z) / CHUNK_X - 1.0f;
	glm::ivec2 pos(cx, cz);
	// not found generate new chunk at player pos
	if (this->terr->world.find(pos) == this->terr->world.end())
		this->terr->updateChunk(pos);
	return (this->terr->world[pos]);
}

bool Player::isGrounded()
{
	glm::vec3 current = this->getPosition();
	int x = (int)floor(current.x) % CHUNK_X;
	int y = floor(current.y);
	int z = (int)floor(current.z) % CHUNK_Z;
	if (x < 0)
		x = CHUNK_X + x;
	if (z < 0)
		z = CHUNK_Z + z;

	Block *b = getChunk()->getBlock(x,y-3,z);
	if (b != NULL && (b->getType() != Blocktype::AIR_BLOCK && b->getType() != Blocktype::WATER_BLOCK))
		return (true);
	if (b == NULL)
		return (true);
	return (false);
}

void Player::jump()
{
	glm::vec3 cur = this->getPosition();
	if (this->isGrounded())
	{
		// this->setPosition(glm::vec3(cur.x, cur.y+0.01f, cur.z)); // temp easy code jump
		this->velocity = -12.0f;
	}
}

void Player::applyGravity(float time)
{
	glm::vec3 current = this->getPosition();
	if (!this->isGrounded() || this->velocity < 0)
	{
		this->velocity += this->gravity * time;
		current.y -= this->velocity * time;
		 // need to add adjustment for not going into block
	}
	else if (this->velocity)
	{
		// can use this current velocity for damage from fall damage
		this->velocity = 0.0f;
	}
	this->setPosition(current);
}


// helper funcs for block breaking
static int signum(int x)
{
		return x == 0 ? 0 : x < 0 ? -1 : 1;
}
static float mod(float value, float modulus)
{
	return fmod(fmod(value, modulus) + modulus,	modulus);
}
static float intbound(float s, float ds)
{
	// Find the smallest positive t such that s+t*ds is an integer.
	if (ds < 0)
	{
		return intbound(-s, -ds);
	}
	else
	{
		s = mod(s, 1);
		// problem is now s+t*ds = 1
		return (1-s)/ds;
	}
}

void Player::leftMouseClickEvent()
{
	// block traversal algorithm http://www.cse.yorku.ca/~amana/research/grid.pdf
	glm::vec3 current = this->getPosition();
	glm::vec3 currentView = this->camera->GetViewVector();
	glm::ivec3 current_voxel((int)floor(current.x) % CHUNK_X,
							floor(current.y),
							(int)floor(current.z) % CHUNK_Z);
	if (current_voxel.x < 0)
		current_voxel.x = CHUNK_X + current_voxel.x;
	if (current_voxel.z < 0)
		current_voxel.z = CHUNK_Z + current_voxel.z;

	// normalized ray direction.
	glm::vec3 ray = this->camera->GetViewVector();

	// in which direction the voxel ids are incremented.
	int stepX = ray.x > 0.0f ? 1 : ray.x < 0.0f ? -1 : 0;
	int stepY = ray.y > 0.0f ? 1 : ray.y < 0.0f ? -1 : 0;
	int stepZ = ray.z > 0.0f ? 1 : ray.z < 0.0f ? -1 : 0;

	// distance until next intersection with voxel-border
	// the value of t at which the ray crosses the first vertical voxel boundary
	float tMaxX = intbound(current.x,ray.x);
	float tMaxY = intbound(current.y,ray.y);
	float tMaxZ = intbound(current.z,ray.z);

	// tDeltaX, tDeltaY, tDeltaZ
	// how far along the ray we must move for the horizontal component to equal the width of a voxel
	// the direction in which we traverse the grid
	float tDeltaX = ((float)stepX) / ray.x;
	float tDeltaY = ((float)stepY) / ray.y;
	float tDeltaZ = ((float)stepZ) / ray.z;
	
	int breakDist = 0;
	Chunk *c = this->getChunk();
	Block *b = c->getBlock(current_voxel.x,current_voxel.y,current_voxel.z);
	while ((!b || !b->isActive()) && breakDist < 50)
	{
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				current_voxel.x += stepX;
				tMaxX += tDeltaX;
			}
			else
			{
				current_voxel.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				current_voxel.y += stepY;
				tMaxY += tDeltaY;
			}
			else
			{
				current_voxel.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}

		// Chunk boundry
		if (current_voxel.x < 0)
		{
			c = c->getXMinus();
			if (!c)
				break ;
			current_voxel.x = CHUNK_X + current_voxel.x;
		}
		else if (current_voxel.x >= CHUNK_X)
		{
			c = c->getXPlus();
			if (!c)
				break ;
			current_voxel.x -= CHUNK_X;
		}

		if (current_voxel.z < 0)
		{
			c = c->getZMinus();
			if (!c)
				break ;
			current_voxel.z = CHUNK_Z + current_voxel.z;
		}
		else if (current_voxel.z >= CHUNK_Z)
		{
			c = c->getZPlus();
			if (!c)
				break ;
			current_voxel.z -= CHUNK_Z;
		}
		b = c->getBlock(current_voxel.x,current_voxel.y,current_voxel.z);
		breakDist++;
	}

	// update the chunks if block is found
	if (b && b->isActive())
	{
		if (b->getType() == Blocktype::LIGHT_BLOCK)
		{
			short val = (short)c->getTorchLight(current_voxel.x,current_voxel.y,current_voxel.z);
			this->terr->lightEngine->lightRemovalBfsQueue.emplace(current_voxel.x,current_voxel.y,current_voxel.z, val, c);
			c->setTorchLight(current_voxel.x,current_voxel.y,current_voxel.z, 0);
			this->terr->lightEngine->removedLighting();
		}
		b->setType(Blocktype::AIR_BLOCK);
		this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()));
		// edge blocks broken require neighbor chunk updates too
		if (current_voxel.x == 0)
		{
			if (c->getXMinus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff()-1, c->getZOff()));
		}
		if (current_voxel.x == CHUNK_X-1)
		{
			if (c->getXPlus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff()+1, c->getZOff()));
		}
		if (current_voxel.z == 0)
		{
			if (c->getZMinus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()-1));
		}
		if (current_voxel.z == CHUNK_Z-1)
		{
			if (c->getZPlus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()+1));
		}
	}
}

void Player::rightMouseClickEvent()
{
	// block traversal algorithm http://www.cse.yorku.ca/~amana/research/grid.pdf
	glm::vec3 current = this->getPosition();
	glm::vec3 currentView = this->camera->GetViewVector();
	glm::ivec3 current_voxel((int)floor(current.x) % CHUNK_X,
							floor(current.y),
							(int)floor(current.z) % CHUNK_Z);
	if (current_voxel.x < 0)
		current_voxel.x = CHUNK_X + current_voxel.x;
	if (current_voxel.z < 0)
		current_voxel.z = CHUNK_Z + current_voxel.z;

	// normalized ray direction.
	glm::vec3 ray = this->camera->GetViewVector();

	// in which direction the voxel ids are incremented.
	int stepX = ray.x > 0.0f ? 1 : ray.x < 0.0f ? -1 : 0;
	int stepY = ray.y > 0.0f ? 1 : ray.y < 0.0f ? -1 : 0;
	int stepZ = ray.z > 0.0f ? 1 : ray.z < 0.0f ? -1 : 0;

	// distance until next intersection with voxel-border
	// the value of t at which the ray crosses the first vertical voxel boundary
	float tMaxX = intbound(current.x,ray.x);
	float tMaxY = intbound(current.y,ray.y);
	float tMaxZ = intbound(current.z,ray.z);

	// tDeltaX, tDeltaY, tDeltaZ
	// how far along the ray we must move for the horizontal component to equal the width of a voxel
	// the direction in which we traverse the grid
	float tDeltaX = ((float)stepX) / ray.x;
	float tDeltaY = ((float)stepY) / ray.y;
	float tDeltaZ = ((float)stepZ) / ray.z;
	
	int breakDist = 0;
	Chunk *c = this->getChunk();
	Block *b = c->getBlock(current_voxel.x,current_voxel.y,current_voxel.z);
	Block *e;
	glm::vec3 vec;
	while ((!b || !b->isActive()) && breakDist < 50)
	{
		vec = glm::vec3(current_voxel.x, current_voxel.y, current_voxel.z);
		e = c->getBlock(current_voxel.x,current_voxel.y,current_voxel.z);
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				current_voxel.x += stepX;
				tMaxX += tDeltaX;
			}
			else
			{
				current_voxel.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				current_voxel.y += stepY;
				tMaxY += tDeltaY;
			}
			else
			{
				current_voxel.z += stepZ;
				tMaxZ += tDeltaZ;
			}
		}

		// Chunk boundry
		if (current_voxel.x < 0)
		{
			c = c->getXMinus();
			if (!c)
				break ;
			current_voxel.x = CHUNK_X + current_voxel.x;
		}
		else if (current_voxel.x >= CHUNK_X)
		{
			c = c->getXPlus();
			if (!c)
				break ;
			current_voxel.x -= CHUNK_X;
		}

		if (current_voxel.z < 0)
		{
			c = c->getZMinus();
			if (!c)
				break ;
			current_voxel.z = CHUNK_Z + current_voxel.z;
		}
		else if (current_voxel.z >= CHUNK_Z)
		{
			c = c->getZPlus();
			if (!c)
				break ;
			current_voxel.z -= CHUNK_Z;
		}
		b = c->getBlock(current_voxel.x,current_voxel.y,current_voxel.z);
		breakDist++;
	}

	// update the chunks if block is found
	if (b && b->isActive() && e && !e->isActive())
	{
		e->setType(this->currentBlockPlace);
		// handle lighting blocks
		if (e->getType() == Blocktype::LIGHT_BLOCK)
		{
			c->setTorchLight(vec.x,vec.y,vec.z,14);
			terr->lightEngine->lightBfsQueue.emplace(vec.x, vec.y, vec.z, c);
			// clear out light queue
			terr->lightEngine->lampLighting();
		}
		this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()));
		// edge blocks broken require neighbor chunk updates too
		if (current_voxel.x == 0)
			if (c->getXMinus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff()-1, c->getZOff()));
		if (current_voxel.x == CHUNK_X-1)
			if (c->getXPlus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff()+1, c->getZOff()));
		if (current_voxel.z == 0)
			if (c->getZMinus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()-1));
		if (current_voxel.z == CHUNK_Z-1)
			if (c->getZPlus()!=NULL)
				this->terr->updateChunk(glm::ivec2(c->getXOff(), c->getZOff()+1));
	}
}