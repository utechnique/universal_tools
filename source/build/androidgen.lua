-- Generates android manifest file.
-- @options members:
--    projname      - name of the project
--    projdir       - location (folder) of the project
--    pkgname       - name of the android package project
--    useinet       - set 'true' to allow internet connection
--    externstorage - set 'true' to allow access to the sd card
function utGenerateAndroidManifest(options)
    filename = BUILD_TARGET .. "/" .. options.projdir .. "/AndroidManifest.xml"
    file = io.open(filename, "w")
    file:write "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    file:write "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
    file:write "                package=\"com.$(ApplicationName)\"\n"
    file:write "                android:versionCode=\"1\"\n"
    file:write "                android:versionName=\"1.0\">\n"
    file:write(string.format("    <uses-sdk android:minSdkVersion=\"%i\" android:targetSdkVersion=\"%i\"/>\n", ANDROID_SDK, ANDROID_TARGET))
    file:write(options.useinet and "    <uses-permission android:name=\"android.permission.INTERNET\"/>\n" or "")
    file:write(options.externstorage and "    <uses-permission android:name=\"android.permission.WRITE_EXTERNAL_STORAGE\"/>\n" or "")
    file:write "    <application android:label=\"@string/app_name\" android:hasCode=\"true\">\n"
    file:write("        <activity android:name=\"." .. options.pkgname .. "\"\n")
    file:write "                  android:noHistory=\"true\"\n"
    file:write "                  android:label=\"@string/app_name\"\n"
    file:write "                  android:screenOrientation=\"portrait\">\n"
    file:write "            <meta-data android:name=\"android.app.lib_name\"\n"
    file:write("                       android:value=\"$(OutDir)/lib" .. options.projname .. ".so\"/>\n")
    file:write "            <intent-filter>\n"
    file:write "                <action android:name=\"android.intent.action.MAIN\"/>\n"
    file:write "                <category android:name=\"android.intent.category.LAUNCHER\"/>\n"
    file:write "            </intent-filter>\n"
    file:write "        </activity>\n"
    file:write "    </application>\n"
    file:write "</manifest> \n"
    file:close()
    return filename
end

-- Generates Ant build script for android project.
--    projdir - location (folder) of the project
function utGenerateAndroidAntScript(projdir)
    filename = BUILD_TARGET .. "/" .. projdir .. "/build.xml"
    file = io.open(filename, "w")
    file:write "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    file:write "<project name=\"$(projectname)\" default=\"help\">\n"
    file:write "    <property file=\"ant.properties\"/>\n"
    file:write "    <property environment=\"env\" />\n"
    file:write "    <condition property=\"sdk.dir\" value=\"${env.ANDROID_HOME}\">\n"
    file:write "        <isset property=\"env.ANDROID_HOME\"/>\n"
    file:write "    </condition>\n"
    file:write "    <loadproperties srcFile=\"project.properties\"/>\n"
    file:write "    <fail\n"
    file:write "        message=\"sdk.dir is missing. Make sure ANDROID_HOME environment variable is correctly set.\"\n"
    file:write "        unless=\"sdk.dir\"\n"
    file:write "    />\n"
    file:write "    <import file=\"custom_rules.xml\" optional=\"true\"/>\n"
    file:write "    <import file=\"${sdk.dir}/tools/ant/build.xml\"/>\n"
    file:write "    <target name=\"-pre-compile\">\n"
    file:write "        <path id=\"project.all.jars.path\">\n"
    file:write "            <path path=\"${toString:project.all.jars.path}\"/>\n"
    file:write "            <fileset dir=\"${jar.libs.dir}\">\n"
    file:write "                <include name=\"*.jar\"/>\n"
    file:write "            </fileset>\n"
    file:write "        </path>\n"
    file:write "    </target>\n"
    file:write "</project>\n"
    file:close()
    return filename
end

-- Generates android properties file.
--    projdir - location (folder) of the project
function utGenerateAndroidProperties(projdir)
    filename = BUILD_TARGET .. "/" .. projdir .. "/project.properties"
    file = io.open(filename, "w")
    file:write "target=$(androidapilevel)\n"
    file:write "$(AntDependencies)\n"
    file:close()
    return filename
end

-- Generates android resource file for strings.
--    projdir - location (folder) of the project
function utGenerateAndroidStringRc(projdir)
    filename = BUILD_TARGET .. "/" .. projdir .. "/res/values/strings.xml"
    file = io.open(filename, "w")
    file:write "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    file:write "<resources>\n"
    file:write "    <string name=\"app_name\">CompatibilityTest</string>\n"
    file:write "</resources>\n"
    file:close()
    return filename
end

-- Generates java glue code for android native application.
-- @options members:
--    projname - name of the project
--    projdir  - location (folder) of the project
--    pkgname  - name of the android package project
function utGenerateAndroidJavaGlue(options)
    -- Filenames of the source and destination files
    srcfilename = "../source/build/android/ConsoleAppGlue.java"
    dstfilename = BUILD_TARGET .. "/" .. options.projdir .. "/src/com/" .. options.pkgname .. "/" .. options.pkgname .. ".java"
    
    -- Load source code from original file
    srcfile = io.open(srcfilename, "r")
    contents = srcfile:read("*a")
    srcfile:close()
    
    -- Replace every '__UT_APP_NAME__' and '__UT_APP_PKG_NAME__' entries
    final_code = contents:gsub("__UT_APP_NAME__", options.projname)
    final_code = final_code:gsub("__UT_APP_PKG_NAME__", options.pkgname)
    
    -- Save final file
    dstfile = io.open(dstfilename, "w")
    dstfile:write(final_code)
    dstfile:close()
    
    return dstfilename
end

-- Generates cpp glue code to communicate with java code for android native application.
-- @options members:
--    projname - name of the project
--    projdir  - location (folder) of the project
--    pkgname  - name of the android package project
function utGenerateAndroidCppGlue(options)
    -- Filenames of the source and destination files
    srcfilename = "../source/build/android/ConsoleAppGlue.cpp"
    dstfilename = BUILD_TARGET .. "/" .. options.projdir .. "/android_glue/ConsoleAppGlue.cpp"
    
    -- Load source code from original file
    srcfile = io.open(srcfilename, "r")
    contents = srcfile:read("*a")
    srcfile:close()
    
    -- Convert every '_' symbol to '_1', because JNI mangles this symbol
    appname_mangled = options.projname:gsub("_", "_1")
    pkgname_mangled = options.pkgname:gsub("_", "_1")
    
    -- Replace every '__UT_APP_NAME__' and '__UT_APP_PKG_NAME__' entries
    final_code = contents:gsub("__UT_APP_NAME__", appname_mangled)
    final_code = final_code:gsub("__UT_APP_PKG_NAME__", pkgname_mangled)
    
    -- Save final file
    dstfile = io.open(dstfilename, "w")
    dstfile:write(final_code)
    dstfile:close()
    
    return dstfilename
end