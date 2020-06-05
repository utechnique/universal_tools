-- check what render api is supported by platform
OPENGL = _OPTIONS["opengl"]
DX11 = _OPTIONS["dx11"]
VULKAN = _OPTIONS["vulkan"]

-- only one render api can be used
if OPENGL then
    DX11 = false
	VULKAN = false
elseif DX11 then
	VULKAN = false
end

-- check if at least one of the render api is supported by platform
SUPPORTS_3D = OPENGL or DX11 or VULKAN

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

elseif VULKAN then
	
	if WINDOWS then
		-- include directories
		table.insert(RENDER_INCLUDE_DIRS, "$(VULKAN_SDK)\\Include")
		
		-- libraries
		table.insert(RENDER_LIB_DIRS_32, "$(VULKAN_SDK)\\Lib32")
		table.insert(RENDER_LIB_DIRS_64, "$(VULKAN_SDK)\\Lib")
		table.insert(RENDER_LIBS, "vulkan-1.lib")
	elseif LINUX then
		table.insert(RENDER_LIBS, "vulkan")
	end	
	
	-- macros
    table.insert(RENDER_DEFS, "VE_VULKAN")
end