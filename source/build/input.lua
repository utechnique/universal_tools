-- input api libraries
INPUT_LIBS = {}

if WINDOWS then
	-- direct input
	table.insert(INPUT_LIBS, "dinput8.lib")
	table.insert(INPUT_LIBS, "dxguid.lib")
elseif LINUX then

end