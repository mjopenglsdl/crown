/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/error/error.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/string_stream.h"
#include "device/device.h"
#include "device/log.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.h"
#include "resource/lua_resource.h"
#include "resource/resource_manager.h"
#include <lua.hpp>
#include <stdarg.h>

LOG_SYSTEM(LUA, "lua")

namespace crown
{
extern void load_api(LuaEnvironment& env);

static int luaB_print(lua_State* L)
{
	TempAllocator2048 ta;
	StringStream ss(ta);

	int n = lua_gettop(L); /* number of arguments */
	lua_getglobal(L, "tostring");
	for (int i = 1; i <= n; ++i)
	{
		const char *s;
		lua_pushvalue(L, -1); /* function to be called */
		lua_pushvalue(L, i);  /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1); /* get result */
		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

		if (i > 1)
			ss << "\t";
		ss << s;
		lua_pop(L, 1); /* pop result */
	}

	logi(LUA, string_stream::c_str(ss));
	return 0;
}

static int msghandler(lua_State* L) {
	const char *msg = lua_tostring(L, 1);
	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
			lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
			return 1;  /* that is the message */
		else
			msg = lua_pushfstring(L, "(error object is a %s value)",
								 luaL_typename(L, 1));
	}
	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}

/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
	int n = lua_gettop(L);
	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK)
			loge(LUA, lua_pushfstring(L, "error calling 'print' (%s)", lua_tostring(L, -1)));
	}
}

/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int addreturn (lua_State *L) {
	const char *line = lua_tostring(L, -1);  /* original line */
	const char *retline = lua_pushfstring(L, "return %s;", line);
	int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
	if (status == LUA_OK)
		lua_remove(L, -2);  /* remove modified line */
	else
		lua_pop(L, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
	return status;
}

/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int loadline (lua_State *L) {
	int status;
	if ((status = addreturn(L)) != LUA_OK) { /* 'return ...' did not work? */
		// status = multiline(L);  /* try as command, maybe with continuation lines */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);  /* get what it has */
		status = luaL_loadbuffer(L, line, len, "=stdin");  /* try it */
	}
	lua_remove(L, 1);  /* remove line from the stack */
	lua_assert(lua_gettop(L) == 1);
	return status;
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
int report (lua_State *L, int status) {
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		loge(LUA, msg);
		lua_pop(L, 1);  /* remove message */
	}
	return status;
}

static int require(lua_State* L)
{
	LuaStack stack(L);
	int status;

	const LuaResource* lr = (LuaResource*)device()->_resource_manager->get(RESOURCE_TYPE_SCRIPT, stack.get_resource_id(1));
	status = luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "");
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}
	return 1;
}

LuaEnvironment::LuaEnvironment()
	: L(NULL)
	, _num_vec3(0)
	, _num_quat(0)
	, _num_mat4(0)
{
	L = luaL_newstate();
	CE_ASSERT(L, "Unable to create lua state");
}

LuaEnvironment::~LuaEnvironment()
{
	lua_close(L);
}

void LuaEnvironment::load_libs()
{
	lua_gc(L, LUA_GCSTOP, 0);

	// Open default libraries
	lua_pushcfunction(L, luaopen_base);
	lua_pushstring(L, "");
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, LUA_LOADLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_table);
	lua_pushstring(L, LUA_TABLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_string);
	lua_pushstring(L, LUA_STRLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_math);
	lua_pushstring(L, LUA_MATHLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_debug);
	lua_pushstring(L, LUA_DBLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_bit);
	lua_pushstring(L, LUA_BITLIBNAME);
	lua_call(L, 1, 0);
	lua_pushcfunction(L, luaopen_jit);
	lua_pushstring(L, LUA_JITLIBNAME);
	lua_call(L, 1, 0);

	// Override print to redirect output to logging system
	add_module_function("_G", "print", luaB_print);

	// Register crown libraries
	load_api(*this);

	// Register custom loader
	lua_getfield(L, LUA_GLOBALSINDEX, "package");
	lua_getfield(L, -1, "loaders");
	lua_pushcfunction(L, require);
	lua_rawseti(L, -2, 1); // package.loaders[1] = require
	lua_pushnil(L);
	lua_rawseti(L, -2, 2); // package.loaders[2] = nil
	lua_pushnil(L);
	lua_rawseti(L, -2, 3); // package.loaders[3] = nil
	lua_pushnil(L);
	lua_rawseti(L, -2, 4); // package.loaders[4] = nil
	lua_pop(L, 2);         // pop package.loaders and package

	// Create metatable for lightuserdata
	lua_pushlightuserdata(L, 0);
	lua_getfield(L, LUA_REGISTRYINDEX, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	// Ensure stack is clean
	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");

	lua_gc(L, LUA_GCRESTART, 0);
}

void LuaEnvironment::require(const char* name)
{
	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	this->call(1, 0);
}

LuaStack LuaEnvironment::execute(const LuaResource* lr, int nres)
{
	LuaStack stack(L);
	int status;

	status = luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "<unknown>");
	if (status == LUA_OK)
		status = this->call(0, nres);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	return stack;
}

LuaStack LuaEnvironment::execute_string(const char* s)
{
	LuaStack stack(L);
	int status;

	status = luaL_loadstring(L, s);
	if (status == LUA_OK)
		status = this->call(0, 0);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
	return stack;
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];
	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(L, module, entry);
	lua_pop(L, 1);
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const char* func)
{
	// Create module if it does not exist
	luaL_Reg entry;
	entry.name = NULL;
	entry.func = NULL;
	luaL_register(L, module, &entry);
	lua_pop(L, 1);

	lua_getglobal(L, module);
	lua_getglobal(L, func);
	lua_setfield(L, -2, name);
	lua_setglobal(L, module);
}

void LuaEnvironment::add_module_metafunction(const char* module, const char* name, const lua_CFunction func)
{
	// Create module if it does not exist
	luaL_Reg entry[2];
	entry[0].name = NULL;
	entry[0].func = NULL;
	luaL_register(L, module, entry);
	lua_pop(L, 1);

	luaL_newmetatable(L, module);
	if (func)
	{
		entry[0].name = name;
		entry[0].func = func;
		entry[1].name = NULL;
		entry[1].func = NULL;
		luaL_register(L, NULL, entry);
	}
	else
	{
		lua_pushstring(L, name);
		lua_pushvalue(L, -2);
		lua_settable(L, -3);
	}

	lua_getglobal(L, module);
	lua_pushvalue(L, -2);
	lua_setmetatable(L, -2);
	lua_pop(L, -1);
}

int LuaEnvironment::call(int narg, int nres)
{
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, msghandler);  /* push message handler */
	lua_insert(L, base);  /* put it under function and args */
	status = lua_pcall(L, narg, nres, base);
	lua_remove(L, base);  /* remove message handler from the stack */
	return status;
}

void LuaEnvironment::call_global(const char* func, int narg, int nres)
{
	int status;
	CE_ENSURE(NULL != func);

	lua_getglobal(L, func);
	lua_insert(L, 1); // Move func to the top of stack
	status = call(narg, nres);
	if (status != LUA_OK)
	{
		report(L, status);
		device()->pause();
	}

	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
}

LuaStack LuaEnvironment::get_global(const char* global)
{
	LuaStack stack(L);
	lua_getglobal(L, global);
	return stack;
}

Vector3* LuaEnvironment::next_vector3(const Vector3& v)
{
	CE_ASSERT(_num_vec3 < LUA_MAX_VECTOR3, "Maximum number of Vector3 reached");
	return &(_vec3[_num_vec3++] = v);
}

Quaternion* LuaEnvironment::next_quaternion(const Quaternion& q)
{
	CE_ASSERT(_num_quat < LUA_MAX_QUATERNION, "Maximum number of Quaternion reached");
	return &(_quat[_num_quat++] = q);
}

Matrix4x4* LuaEnvironment::next_matrix4x4(const Matrix4x4& m)
{
	CE_ASSERT(_num_mat4 < LUA_MAX_MATRIX4X4, "Maximum number of Matrix4x4 reached");
	return &(_mat4[_num_mat4++] = m);
}

bool LuaEnvironment::is_vector3(const Vector3* p) const
{
	return p >= &_vec3[0]
		&& p <= &_vec3[LUA_MAX_VECTOR3 - 1];
}

bool LuaEnvironment::is_quaternion(const Quaternion* p) const
{
	return p >= &_quat[0]
		&& p <= &_quat[LUA_MAX_QUATERNION - 1];
}

bool LuaEnvironment::is_matrix4x4(const Matrix4x4* p) const
{
	return p >= &_mat4[0]
		&& p <= &_mat4[LUA_MAX_MATRIX4X4 - 1];
}

void LuaEnvironment::temp_count(u32& num_vec3, u32& num_quat, u32& num_mat4)
{
	num_vec3 = _num_vec3;
	num_quat = _num_quat;
	num_mat4 = _num_mat4;
}

void LuaEnvironment::set_temp_count(u32 num_vec3, u32 num_quat, u32 num_mat4)
{
	_num_vec3 = num_vec3;
	_num_quat = num_quat;
	_num_mat4 = num_mat4;
}

void LuaEnvironment::reset_temporaries()
{
	_num_vec3 = 0;
	_num_quat = 0;
	_num_mat4 = 0;
}

static void console_command_script(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["script"], script);

	((LuaEnvironment*)user_data)->execute_string(script.c_str());
}

static void do_REPL(LuaEnvironment* env, const char* lua)
{
	lua_State* L = env->L;
	int status;

	lua_settop(L, 0);
	lua_pushstring(L, lua);
	if ((status = loadline(L)) != -1) // This is never -1
	{
		if (status == LUA_OK)
		{
			status = env->call(0, LUA_MULTRET);
		}
		if (status == LUA_OK)
			l_print(L);
		else
			report(L, status);
	}
	lua_settop(L, 0); /* clear stack */
	return;
}

static void console_command_REPL(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["repl"], script);

	do_REPL((LuaEnvironment*)user_data, script.c_str());
}

void LuaEnvironment::register_console_commands(ConsoleServer& cs)
{
	cs.register_command("script", console_command_script, this);
	cs.register_command("repl", console_command_REPL, this);
}

} // namespace crown
