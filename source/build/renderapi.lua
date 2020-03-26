-- check what render api is supported by platform
OPENGL = _OPTIONS["opengl"]
DX11 = _OPTIONS["dx11"]

-- only one render api can be used
if OPENGL and DX11 then
    DX11 = false
end

-- check if at least one of the render api is supported by platform
SUPPORTS_3D = OPENGL or DX11

-- include directories for render api
RENDER_INCLUDE_DIRS = {}

-- library directories for render api
RENDER_LIB_DIRS = {} -- for both 32 and 64
RENDER_LIB_DIRS_32 = {} -- for 32
RENDER_LIB_DIRS_64 = {} -- for 64

-- libraries for render api
RENDER_LIBS = {}
RENDER_LIBS_RELEASE = {}
RENDER_LIBS_DBG = {}

-- macros for render api
RENDER_DEFS = {}

-- initialize opengl data
if OPENGL then
	-- libraries
	if WINDOWS then
		table.insert(RENDER_LIBS, "opengl32.lib")
		table.insert(RENDER_LIBS, "glu32.lib")
	elseif LINUX then
		table.insert(RENDER_LIBS, "GL")
		table.insert(RENDER_LIBS, "GLU")
	end
	
	-- include directories
	table.insert(RENDER_INCLUDE_DIRS, "../contrib/opengl/include")
	
	-- macros
    table.insert(RENDER_DEFS, "VE_OPENGL")
	
elseif DX11 then
	-- include directories
	table.insert(RENDER_INCLUDE_DIRS, "$(DXSDK_DIR)Include")

	-- libraries
	table.insert(RENDER_LIB_DIRS_32, "$(DXSDK_DIR)Lib\\x86")
	table.insert(RENDER_LIB_DIRS_64, "$(DXSDK_DIR)Lib\\x64")
	table.insert(RENDER_LIBS, "d3d11.lib")
	table.insert(RENDER_LIBS, "dxgi.lib")
	
	-- macros
    table.insert(RENDER_DEFS, "VE_DX11")
end