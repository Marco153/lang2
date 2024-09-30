#include "compile.cpp"

int main(int argc, char* argv[])
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


	InitLang(&lang_stat, (AllocTypeFunc)heap_alloc, (FreeTypeFunc)heap_free, &alloc);

	compile_options opts = {};
	opts.file = "../game/files";
	opts.wasm_dir = "../web/";
	if (argc > 1)
	{
		own_std::string is_release = argv[1];
		if (is_release == "0")
		{
			opts.release = false;
		}
		else
		{
			opts.release = true;
		}
	}
	else
	{
		printf("At the moment, Liz only compiles to wasm and you shold specify a folder to compile all files within.\n");
		printf("-f path to source files folder\n");
		printf("-o output folder\n");
		printf("-z generates .dbg files for the interpreter\n");
		return 0;

	}
	if (opts.wasm_dir[opts.wasm_dir.size() - 1] != '/' && opts.wasm_dir.size() != 0)
		opts.wasm_dir += '/';

	Compile(&lang_stat, &opts);

	if (!opts.release)
	{
		long long args[] = { 0 };

		AssignDbgFile(&lang_stat, (opts.wasm_dir + opts.folder_name + ".dbg").c_str());
		//RunDbgFile(&lang_stat, "tests", args, 1);
		//lang_stat.winterp->dbg->data = (void*)&gl_state;
		RunDbgFile(&lang_stat, "main", args, 1);

		int ret_val = *(int *)&lang_stat.winterp->dbg->mem_buffer[RET_1_REG * 8];
	}
	int a = 0;

}
