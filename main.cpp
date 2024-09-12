#include "compile.cpp"
#include "memory.cpp"
#include <glad/glad.h> 
#include <glad/glad.c> 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> 

#include "../include/GLFW/glfw3.h"

#define KEY_HELD 1
#define KEY_DOWN 2
#define KEY_UP   4
#define KEY_RECENTLY_DOWN   8

#define TOTAL_KEYS   255
#define TOTAL_TEXTURES   32

void GetMem(dbg_state* dbg);
struct texture_info
{
	bool used;
	int id;
};
struct open_gl_state
{
	int vao;
	int shader_program;
	int shader_program_no_texture;
	int color_u;
	int pos_u;
	int buttons[TOTAL_KEYS];
	texture_info textures[TOTAL_TEXTURES];
	double last_time;

	int width;
	int height;

	void* glfw_window;
};
enum key_enum
{
	_KEY_LEFT,
	_KEY_RIGHT,
	_KEY_DOWN,
	_KEY_UP,
	_KEY_ACT0,
	_KEY_ACT1,
	_KEY_ACT2,
	_KEY_ACT3,
};

void Print(dbg_state* dbg)
{
	int base = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int mem_alloc_addr = *(int*)&dbg->mem_buffer[base + 8];

	int a = 0;


}
#define DRAW_INFO_HAS_TEXTURE 1
struct draw_info
{
	float pos_x;
	float pos_y;
	float pos_z;

	float pivot_x;
	float pivot_y;
	float pivot_z;

	float ent_size_x;
	float ent_size_y;
	float ent_size_z;

	float color_r;
	float color_g;
	float color_b;
	float color_a;


	int texture_id;

	float cam_size;

	float cam_pos_x;
	float cam_pos_y;
	float cam_pos_z;

	int flags;
};

int GetTextureSlotId(open_gl_state* gl_state)
{
	for (int i = 0; i < TOTAL_TEXTURES; i++)
	{
		texture_info* t = &gl_state->textures[i];
		if (!t->used)
		{
			t->used = true;
			return i;
		}
	}
	ASSERT(0);
}
#define GL_CALL(call) call; if(glGetError() != GL_NO_ERROR) {ASSERT(0)}
void Draw(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int draw_addr = *(int *)&dbg->mem_buffer[base_ptr + 8 * 2];


	auto wnd = (GLFWwindow *)*(long long*)&dbg->mem_buffer[base_ptr + 8];
	auto draw = (draw_info *)(long long*)&dbg->mem_buffer[draw_addr];

	auto gl_state = (open_gl_state*)dbg->data;

	int prog = gl_state->shader_program;

	if (IS_FLAG_ON(draw->flags, DRAW_INFO_HAS_TEXTURE))
	{
		prog = gl_state->shader_program;
		//draw->flags &= ~DRAW_INFO_HAS_TEXTURE;
		texture_info* t = &gl_state->textures[draw->texture_id];

		glBindTexture(GL_TEXTURE_2D, t->id);
	}
	else
	{
		prog = gl_state->shader_program_no_texture;
		//glDisable(GL_TEXTURE_2D);
	}

	glUseProgram(prog);
	glBindVertexArray(gl_state->vao);

	gl_state->color_u = glGetUniformLocation(prog, "color");
	glUniform4f(gl_state->color_u, draw->color_r, draw->color_b, draw->color_g, 1.0f);

	gl_state->pos_u = glGetUniformLocation(prog, "pos");
	glUniform3f(gl_state->pos_u, draw->pos_x, draw->pos_y, draw->pos_z);

	int pivot_u = glGetUniformLocation(prog, "pivot");
	glUniform3f(pivot_u, draw->pivot_x, draw->pivot_y, draw->pivot_z);

	int cam_size_u = glGetUniformLocation(prog, "cam_size");
	glUniform1f(cam_size_u, draw->cam_size);

	float screen_ratio = (float)gl_state->height / (float)gl_state->width;
	int screen_ratio_u = glGetUniformLocation(prog, "screen_ratio");
	glUniform1f(screen_ratio_u, screen_ratio);

	draw->cam_pos_x = 3;
	draw->cam_pos_y = -1;
	int cam_pos_u = glGetUniformLocation(prog, "cam_pos");
	glUniform3f(cam_pos_u, draw->cam_pos_x, draw->cam_pos_y, draw->cam_pos_z);

	int ent_size_u = glGetUniformLocation(prog, "ent_size");
	glUniform3f(ent_size_u, draw->ent_size_x, draw->ent_size_y, draw->ent_size_z);



	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	//*(int*)&dbg->mem_buffer[RET_1_REG * 8] = glfwWindowShouldClose((GLFWwindow *)(long long)wnd);
}
void ClearBackground(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	void* addr = &dbg->mem_buffer[base_ptr + 8];
	float r = *(float*)&dbg->mem_buffer[base_ptr + 8];
	float g = *(float*)&dbg->mem_buffer[base_ptr + 8 * 2];
	float b = *(float*)&dbg->mem_buffer[base_ptr + 8 * 3];

	glClearColor(r, g, b, 1.0f);
	glClearDepth(1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//*(int*)&dbg->mem_buffer[RET_1_REG * 8] = glfwWindowShouldClose((GLFWwindow *)(long long)wnd);
}

int FromGameToGLFWKey(int in)
{
	int key;
	switch ((key_enum)in)
	{
	case _KEY_UP:
	{
		key = GLFW_KEY_W;
	}break;
	case _KEY_DOWN:
	{
		key = GLFW_KEY_S;
	}break;
	case _KEY_RIGHT:
	{
		key = GLFW_KEY_D;
	}break;
	case _KEY_LEFT:
	{
		key = GLFW_KEY_A;
	}break;
	default:
		ASSERT(0);
	}
	return key;
}
void IsKeyDown(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int key = *(int*)&dbg->mem_buffer[base_ptr + 8];
	
	auto gl_state = (open_gl_state*)dbg->data;

	key = FromGameToGLFWKey(key);
	int* addr = (int*)&dbg->mem_buffer[RET_1_REG * 8];

	if (IS_FLAG_ON(gl_state->buttons[key], KEY_DOWN) || IS_FLAG_ON(gl_state->buttons[key], KEY_RECENTLY_DOWN))
	{
		*addr = 1;
		gl_state->buttons[key] &= ~KEY_RECENTLY_DOWN;
		gl_state->buttons[key] = (gl_state->buttons[key] & 0xffff);
	}
	else
		*addr = 0;

}
void IsKeyHeld(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int key = *(int*)&dbg->mem_buffer[base_ptr + 8];
	
	auto gl_state = (open_gl_state*)dbg->data;

	key = FromGameToGLFWKey(key);

	int* addr = (int*)&dbg->mem_buffer[RET_1_REG * 8];

	if (IS_FLAG_ON(gl_state->buttons[key], KEY_HELD))
	{
		*addr = 1;
	}
	else
		*addr = 0;

}
void GetDeltaTime(dbg_state* dbg)
{
	auto gl_state = (open_gl_state*)dbg->data;

	auto ret = (float *)&dbg->mem_buffer[RET_1_REG * 8];
	*ret = glfwGetTime() - gl_state->last_time;
	gl_state->last_time = glfwGetTime();
}
void EndFrame(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int draw_addr = *(int *)&dbg->mem_buffer[base_ptr + 8 * 2];


	auto wnd = (GLFWwindow *)*(long long*)&dbg->mem_buffer[base_ptr + 8];
	auto gl_state = (open_gl_state*)dbg->data;
	//gl_state->last_time = glfwGetTime();
	glfwSwapBuffers(wnd);
}
void ShouldClose(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	long long wnd = *(long long*)&dbg->mem_buffer[base_ptr + 8];

	*(int*)&dbg->mem_buffer[RET_1_REG * 8] = glfwWindowShouldClose((GLFWwindow *)(long long)wnd);



	auto gl_state = (open_gl_state*)dbg->data;

	for (int i = 0; i < TOTAL_KEYS; i++)
	{
		int retain_flags = gl_state->buttons[i] & KEY_HELD;
		gl_state->buttons[i] &= ~(KEY_DOWN | KEY_UP);
		gl_state->buttons[i] |= retain_flags;

		if (IS_FLAG_ON(gl_state->buttons[i], KEY_RECENTLY_DOWN))
		{
			unsigned short held_from = (unsigned short)(gl_state->buttons[i] >> 16);
			held_from++;
			if (held_from > 24)
			{
				gl_state->buttons[i] &= ~KEY_RECENTLY_DOWN;
				gl_state->buttons[i] = (gl_state->buttons[i] & 0xffff);
			}
			else
			{
				gl_state->buttons[i] = (gl_state->buttons[i] & 0xffff) | (held_from << 16);
			}
		}
	}
	
	glfwPollEvents();
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto gl_state = (open_gl_state*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS)
	{
		gl_state->buttons[key] |= KEY_HELD | KEY_DOWN | KEY_RECENTLY_DOWN;
		//printf("key(%d) is %d", key, gl_state->buttons[key]);
	}
	else if (action == GLFW_RELEASE)
	{
		gl_state->buttons[key] &= ~KEY_HELD;
		gl_state->buttons[key] |= KEY_UP;
	}
}
struct clip
{
	unsigned int* texs_idxs;
	unsigned int total_texs;
	unsigned int id;
	float len;
	float cur_time;
	bool loop;
};
struct load_clip_args
{
	unsigned long long ctx;
	unsigned char* file_name;
	unsigned long long x_offset;
	unsigned long long y_offset;
	unsigned long long sp_width;
	unsigned long long sp_height;
	unsigned long long total_sps;
	float len;
	clip* cinfo;
};
void LoadClip(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];

	auto info = (load_clip_args*)&dbg->mem_buffer[base_ptr + 8];
	info->file_name = (unsigned char*)&dbg->mem_buffer[(long long)info->file_name];
	info->cinfo = (clip*)&dbg->mem_buffer[(long long)info->cinfo];
	info->cinfo->total_texs = info->total_sps;
	info->cinfo->len = info->len;

	auto gl_state = (open_gl_state*)dbg->data;

	*(int*)&dbg->mem_buffer[STACK_PTR_REG * 8] -= 16;
	*(int*)&dbg->mem_buffer[STACK_PTR_REG * 8 + 8] = info->total_sps * sizeof(int);
	int idx = 0;
	
	GetMem(dbg);
	int offset = *(int*)&dbg->mem_buffer[RET_1_REG * 8];

	*(int*)&dbg->mem_buffer[STACK_PTR_REG * 8] += 16;

	*(int**)&info->cinfo->texs_idxs = (int*)(long long)offset;
	info->cinfo->total_texs = info->total_sps;

	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* src = stbi_load((char *)info->file_name, &width, &height, &nrChannels, 0);

	auto texs_id = (int*)&dbg->mem_buffer[(long long)info->cinfo->texs_idxs];

	for (int cur_sp = 0; cur_sp < info->total_sps; cur_sp++)
	{
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		int sp_height = info->sp_width;
		int sp_width = info->sp_height;
		auto sp_data = (unsigned char*)AllocMiscData(dbg->lang_stat, sp_width * sp_width * 4);
		int sp_idx = cur_sp;

		for (int i = 0; i < sp_width; i++)
		{
			int x_offset = info->x_offset + sp_idx * sp_width * 4;
			//int y_offset = sp_idx * sp_height;
			memcpy(sp_data + i * sp_width * 4, src + x_offset + (i * width * 4), sp_width * 4);
		}
		if (sp_data)
		{
			GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sp_width, sp_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, sp_data));
			//GL_CALL(glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0));

			GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		int idx = GetTextureSlotId(gl_state);
		texture_info* tex = &gl_state->textures[idx];
		tex->id = texture;

		heap_free((mem_alloc *)__lang_globals.data, (char *)sp_data);

		texs_id[cur_sp] = idx;

	}
	stbi_image_free(src);
	/*
	func_decl* call_f = FuncAddedWasmInterp(dbg->wasm_state, "heap_alloc");

	block_linked* cur = NewBlock(nullptr);
	WasmDoCallInstruction(dbg, dbg->cur_bc, &cur, call_f);
	FreeBlock(cur);
	int addr = *(int*)&dbg->mem_buffer[RET_1_REG * 8];
	//dbg->wasm_state->funcs
	int a = 0;
	*/
}

int CompileShader(char* source, int type)
{
	int  success;
	char infoLog[512];
	unsigned int shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
 
	//ASSERT(gl_)

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return shader;
}
void UpdateLastTime(dbg_state* dbg)
{
	auto gl_state = (open_gl_state*)dbg->data;
	if(gl_state)
		gl_state->last_time = glfwGetTime();

}
void OpenWindow(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int sz = *(int*)&dbg->mem_buffer[base_ptr + 8];

	auto gl_state = (open_gl_state*)dbg->data;

	GLFWwindow* window;


	/* Initialize the library */
	if (!glfwInit())
		return;

	gl_state->width = 1200;
	gl_state->height = 1000;
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(gl_state->width, gl_state->height, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}
	gl_state->glfw_window = window;

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetWindowUserPointer(window, (void*)gl_state);
	glfwSetKeyCallback(window, KeyCallback);

	*(long long*)&dbg->mem_buffer[RET_1_REG * 8] = (long long )window;
	float vertices[] = {
		// positions          // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 1.0f,   // top right
		 1.0f,   0.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		 0.0f,	0.0f, 0.0f,   0.0f, 0.0f,   // bottom left
		 0.0f,  1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
	GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 uv;\n"
		"uniform vec3 pos;\n"
		"uniform vec3 pivot;\n"
		"uniform float cam_size;\n"
		"uniform float screen_ratio;\n"
		"uniform vec3 ent_size;\n"
		"uniform vec3 cam_pos;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"   gl_Position.xy -= pivot.xy;\n"
		"   gl_Position.xy *= ent_size.xy;\n"
		"   gl_Position.xy += pos.xy;\n"
		"   gl_Position.xy -= cam_pos.xy;\n"
		"   gl_Position.xy /= cam_size;\n"
		"   gl_Position.x *= screen_ratio;\n"
		//"   gl_Position = ition / cam_size + cam_size;\n"
		"   TexCoord = uv;\n"
		"}\0";

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GL_CALL(glShaderSource(vertexShader, 1, &vertexShaderSource, NULL));
	GL_CALL(glCompileShader(vertexShader));

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	const char* fragmentShaderNoTextureSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform vec4 color;\n"
		"void main(){\n"
		//"vec4 tex_col =  texture(tex, uv);\n"
		"FragColor =  color;\n"
		"}\n";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform vec4 color;\n"
		"uniform sampler2D tex;\n"
		"void main(){\n"
		"vec4 tex_col =  texture(tex, TexCoord);\n"
		//"vec4 tex_col =  texture(tex, uv);\n"
		"FragColor =  tex_col * color;\n"
		"}\n";

	unsigned int fragmentShader = CompileShader((char *)fragmentShaderSource, GL_FRAGMENT_SHADER);
	unsigned int fragmentNoTextureShader = CompileShader((char *)fragmentShaderNoTextureSource, GL_FRAGMENT_SHADER);


	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	unsigned int shaderProgramNoTexture;
	shaderProgramNoTexture = glCreateProgram();
	glAttachShader(shaderProgramNoTexture, vertexShader);
	glAttachShader(shaderProgramNoTexture, fragmentNoTextureShader);
	glLinkProgram(shaderProgramNoTexture);
	//glUseProgram(shaderProgram);


	gl_state->vao = VAO;
	gl_state->shader_program = shaderProgram;
	gl_state->shader_program_no_texture = shaderProgramNoTexture;


	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);



	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load and generate the texture
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load("../images/frog.png", &width, &height, &nrChannels, 0);

	int sp_height = 32;
	int sp_width = 32;
	auto sp_data = (unsigned char* )AllocMiscData(dbg->lang_stat, 32 * 32 * 4);
	int sp_idx = 1;

	for (int i = 0; i < sp_width; i++)
	{
		int x_offset = sp_idx * sp_width * 4;
		//int y_offset = sp_idx * sp_height;
		memcpy(sp_data + i * sp_width * 4, data + x_offset + (i * width * 4), sp_width * 4);
	}
	stbi_image_free(data);
	data = sp_data;
	if (data)
	{
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sp_width, sp_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
		//GL_CALL(glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0));

		GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	int idx = GetTextureSlotId(gl_state);
	texture_info* tex = &gl_state->textures[idx];
	tex->id = texture;


}
/*
void DebuggerCommand(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int str_offset = *(int*)&dbg->mem_buffer[base_ptr + 8];
	//ASSERT(sz > 0)

	char* str = (char*)&dbg->mem_buffer[str_offset];


	int addr = *(int*)&dbg->mem_buffer[MEM_PTR_CUR_ADDR];
	//int *max = (int*)&dbg->mem_buffer[MEM_PTR_MAX_ADDR];
	*(int*)&dbg->mem_buffer[MEM_PTR_CUR_ADDR] += sz;
	ASSERT((addr + sz) < 64000);
	//*max += sz;

	*(int*)&dbg->mem_buffer[RET_1_REG * 8] = addr;

}
*/
void GetMem(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	int sz = *(int*)&dbg->mem_buffer[base_ptr + 8];
	ASSERT(sz > 0)

	int addr = *(int*)&dbg->mem_buffer[MEM_PTR_CUR_ADDR];
	//int *max = (int*)&dbg->mem_buffer[MEM_PTR_MAX_ADDR];
	*(int*)&dbg->mem_buffer[MEM_PTR_CUR_ADDR] += sz;
	ASSERT((addr + sz) < 64000);
	//*max += sz;

	*(int*)&dbg->mem_buffer[RET_1_REG * 8] = addr;

}

void GetTimeSinceStart(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	float val = *(float*)&dbg->mem_buffer[base_ptr + 8];
	//auto gl_state = (open_gl_state*)dbg->data;

	*(float*)&dbg->mem_buffer[RET_1_REG * 8] = (float)glfwGetTime();
}
void Sqrt(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	float val = *(float*)&dbg->mem_buffer[base_ptr + 8];

	*(float*)&dbg->mem_buffer[RET_1_REG * 8] = sqrt(val);
}
void Sin(dbg_state* dbg)
{
	int base_ptr = *(int*)&dbg->mem_buffer[STACK_PTR_REG * 8];
	float val = *(float*)&dbg->mem_buffer[base_ptr + 8];

	*(float*)&dbg->mem_buffer[RET_1_REG * 8] = sinf(val);
}
int main()
{
	lang_state lang_stat;
	mem_alloc alloc;
	InitMemAlloc(&alloc);
	/*
	auto addr = heap_alloc(&alloc, 12);
	auto addr2 = heap_alloc(&alloc, 12);
	heap_free(&alloc, addr);
	heap_free(&alloc, addr2);
	*/


	open_gl_state gl_state = {};
	InitLang(&lang_stat, (AllocTypeFunc)heap_alloc, (FreeTypeFunc)heap_free, &alloc);
	compile_options opts = {};
	opts.file = "../lang2/files";
	opts.wasm_dir = "../../wabt/";
	opts.release = false;

	AssignOutsiderFunc(&lang_stat, "GetMem", (OutsiderFuncType)GetMem);
	AssignOutsiderFunc(&lang_stat, "Print", (OutsiderFuncType)Print);
	AssignOutsiderFunc(&lang_stat, "OpenWindow", (OutsiderFuncType)OpenWindow);
	AssignOutsiderFunc(&lang_stat, "ShouldClose", (OutsiderFuncType)ShouldClose);
	AssignOutsiderFunc(&lang_stat, "ClearBackground", (OutsiderFuncType)ClearBackground);
	AssignOutsiderFunc(&lang_stat, "Draw", (OutsiderFuncType)Draw);
	AssignOutsiderFunc(&lang_stat, "IsKeyHeld", (OutsiderFuncType)IsKeyHeld);
	AssignOutsiderFunc(&lang_stat, "IsKeyDown", (OutsiderFuncType)IsKeyDown);
	AssignOutsiderFunc(&lang_stat, "LoadClip", (OutsiderFuncType)LoadClip);
	AssignOutsiderFunc(&lang_stat, "GetDeltaTime", (OutsiderFuncType)GetDeltaTime);
	AssignOutsiderFunc(&lang_stat, "EndFrame", (OutsiderFuncType)EndFrame);
	AssignOutsiderFunc(&lang_stat, "GetTimeSinceStart", (OutsiderFuncType)GetTimeSinceStart);
	AssignOutsiderFunc(&lang_stat, "sqrt", (OutsiderFuncType)Sqrt);
	//AssignOutsiderFunc(&lang_stat, "DebuggerCommand", (OutsiderFuncType)DebuggerCommand);
	AssignOutsiderFunc(&lang_stat, "sin", (OutsiderFuncType)Sin);
	Compile(&lang_stat, &opts);
	if (!opts.release)
	{
		long long args[] = { 0 };

		AssignDbgFile(&lang_stat, "../../wabt/dbg_wasm.dbg");
		RunDbgFile(&lang_stat, "tests", args, 1);
		lang_stat.winterp->dbg->data = (void*)&gl_state;
		RunDbgFile(&lang_stat, "main", args, 1);

		int ret_val = *(int *)&lang_stat.winterp->dbg->mem_buffer[RET_1_REG * 8];
	}
	int a = 0;

}

