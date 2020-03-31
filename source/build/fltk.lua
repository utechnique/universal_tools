FLTK_INCLUDE_DIR =
{
    "../source/fltk",
    "../contrib/fltk",
    "../contrib/fltk/zlib",
    "../contrib/fltk/png",
    "../contrib/fltk/jpeg",
}

FLTK_WINDOWS_DEF =
{
    "WIN32",
    "_WINDOWS",
    "WIN32_LEAN_AND_MEAN",
    "WIN32_EXTRA_LEAN",
    "VC_EXTRA_LEAN",
}

FLTK_DEF =
{
    "_CRT_SECURE_NO_DEPRECATE",
}

FLTK_PLATFORM_DEF = WINDOWS and FLTK_WINDOWS_DEF or {}

FLTK_LIBS = LINUX and { "X11", "dl" } or {}

if OPENGL then
	FLTK_GL_SRC = 
	{
		"../contrib/fltk/src/Fl_Gl_Choice.cxx",
		"../contrib/fltk/src/Fl_Gl_Device_Plugin.cxx",
		"../contrib/fltk/src/Fl_Gl_Overlay.cxx",
		"../contrib/fltk/src/Fl_Gl_Window.cxx",
		"../contrib/fltk/src/gl_draw.cxx",
		"../contrib/fltk/src/gl_start.cxx",
	}
else
	FLTK_GL_SRC = {} 
end

function utFLTK()    
    -- fltk
    utStaticLibProj
    {
        projname = "fltk",     -- project name
        projdir  = "fltk",     -- project folder
        incdir   =             -- include
        {
            FLTK_INCLUDE_DIR,
        },
        def      =             -- defines
        {
            FLTK_DEF,
            FLTK_PLATFORM_DEF,
            "FL_LIBRARY"
        },
        charset  = "MBCS",     -- character set
        srcfiles =             -- sources
        {
            -- .cpp files
            "../contrib/fltk/src/Fl.cxx",
            "../contrib/fltk/src/Fl_Adjuster.cxx",
            "../contrib/fltk/src/Fl_Bitmap.cxx",
            "../contrib/fltk/src/Fl_Browser.cxx",
            "../contrib/fltk/src/Fl_Browser_.cxx",
            "../contrib/fltk/src/Fl_Browser_load.cxx",
            "../contrib/fltk/src/Fl_Box.cxx",
            "../contrib/fltk/src/Fl_Button.cxx",
			"../contrib/fltk/src/fl_call_main.c",
            "../contrib/fltk/src/Fl_Chart.cxx",
            "../contrib/fltk/src/Fl_Check_Browser.cxx",
            "../contrib/fltk/src/Fl_Check_Button.cxx",
            "../contrib/fltk/src/Fl_Choice.cxx",
            "../contrib/fltk/src/Fl_Clock.cxx",
            "../contrib/fltk/src/Fl_Color_Chooser.cxx",
            "../contrib/fltk/src/Fl_Copy_Surface.cxx",
            "../contrib/fltk/src/Fl_Counter.cxx",
            "../contrib/fltk/src/Fl_Device.cxx",
            "../contrib/fltk/src/Fl_Dial.cxx",
            "../contrib/fltk/src/Fl_Help_Dialog_Dox.cxx",
            "../contrib/fltk/src/Fl_Double_Window.cxx",
            "../contrib/fltk/src/Fl_File_Browser.cxx",
            "../contrib/fltk/src/Fl_File_Chooser.cxx",
            "../contrib/fltk/src/Fl_File_Chooser2.cxx",
            "../contrib/fltk/src/Fl_File_Icon.cxx",
            "../contrib/fltk/src/Fl_File_Input.cxx",
            "../contrib/fltk/src/Fl_Group.cxx",
            "../contrib/fltk/src/Fl_Help_View.cxx",
            "../contrib/fltk/src/Fl_Image.cxx",
            "../contrib/fltk/src/Fl_Image_Surface.cxx",
            "../contrib/fltk/src/Fl_Input.cxx",
            "../contrib/fltk/src/Fl_Input_.cxx",
            "../contrib/fltk/src/Fl_Light_Button.cxx",
            "../contrib/fltk/src/Fl_Menu.cxx",
            "../contrib/fltk/src/Fl_Menu_.cxx",
            "../contrib/fltk/src/Fl_Menu_Bar.cxx",
            "../contrib/fltk/src/Fl_Menu_Button.cxx",
            "../contrib/fltk/src/Fl_Menu_Window.cxx",
            "../contrib/fltk/src/Fl_Menu_add.cxx",
            "../contrib/fltk/src/Fl_Menu_global.cxx",
            "../contrib/fltk/src/Fl_Multi_Label.cxx",
            "../contrib/fltk/src/Fl_Native_File_Chooser.cxx",
            "../contrib/fltk/src/Fl_Overlay_Window.cxx",
            "../contrib/fltk/src/Fl_Pack.cxx",
            "../contrib/fltk/src/Fl_Paged_Device.cxx",
            "../contrib/fltk/src/Fl_Pixmap.cxx",
            "../contrib/fltk/src/Fl_Positioner.cxx",
            "../contrib/fltk/src/Fl_PostScript.cxx",
            "../contrib/fltk/src/Fl_Printer.cxx",
            "../contrib/fltk/src/Fl_Preferences.cxx",
            "../contrib/fltk/src/Fl_Progress.cxx",
            "../contrib/fltk/src/Fl_Repeat_Button.cxx",
            "../contrib/fltk/src/Fl_Return_Button.cxx",
            "../contrib/fltk/src/Fl_Roller.cxx",
            "../contrib/fltk/src/Fl_Round_Button.cxx",
            "../contrib/fltk/src/Fl_Scroll.cxx",
            "../contrib/fltk/src/Fl_Scrollbar.cxx",
            "../contrib/fltk/src/Fl_Shared_Image.cxx",
            "../contrib/fltk/src/Fl_Single_Window.cxx",
            "../contrib/fltk/src/Fl_Slider.cxx",
            "../contrib/fltk/src/Fl_Table.cxx",
            "../contrib/fltk/src/Fl_Table_Row.cxx",
            "../contrib/fltk/src/Fl_Tabs.cxx",
            "../contrib/fltk/src/Fl_Text_Buffer.cxx",
            "../contrib/fltk/src/Fl_Text_Display.cxx",
            "../contrib/fltk/src/Fl_Text_Editor.cxx",
            "../contrib/fltk/src/Fl_Tile.cxx",
            "../contrib/fltk/src/Fl_Tiled_Image.cxx",
            "../contrib/fltk/src/Fl_Tooltip.cxx",
            "../contrib/fltk/src/Fl_Tree.cxx",
            "../contrib/fltk/src/Fl_Tree_Item_Array.cxx",
            "../contrib/fltk/src/Fl_Tree_Item.cxx",
            "../contrib/fltk/src/Fl_Tree_Prefs.cxx",
            "../contrib/fltk/src/Fl_Valuator.cxx",
            "../contrib/fltk/src/Fl_Value_Input.cxx",
            "../contrib/fltk/src/Fl_Value_Output.cxx",
            "../contrib/fltk/src/Fl_Value_Slider.cxx",
            "../contrib/fltk/src/Fl_Widget.cxx",
            "../contrib/fltk/src/Fl_Window.cxx",
            "../contrib/fltk/src/Fl_Window_fullscreen.cxx",
            "../contrib/fltk/src/Fl_Window_hotspot.cxx",
            "../contrib/fltk/src/Fl_Window_iconize.cxx",
            "../contrib/fltk/src/Fl_Window_shape.cxx",
            "../contrib/fltk/src/Fl_Wizard.cxx",
            "../contrib/fltk/src/Fl_XBM_Image.cxx",
            "../contrib/fltk/src/Fl_XPM_Image.cxx",
            "../contrib/fltk/src/Fl_abort.cxx",
            "../contrib/fltk/src/Fl_add_idle.cxx",
            "../contrib/fltk/src/Fl_arg.cxx",
            "../contrib/fltk/src/Fl_compose.cxx",
            "../contrib/fltk/src/Fl_display.cxx",
            "../contrib/fltk/src/Fl_get_key.cxx",
            "../contrib/fltk/src/Fl_get_system_colors.cxx",
            "../contrib/fltk/src/Fl_grab.cxx",
            "../contrib/fltk/src/Fl_lock.cxx",
            "../contrib/fltk/src/Fl_own_colormap.cxx",
            "../contrib/fltk/src/Fl_visual.cxx",
            "../contrib/fltk/src/Fl_x.cxx",
            "../contrib/fltk/src/filename_absolute.cxx",
            "../contrib/fltk/src/filename_expand.cxx",
            "../contrib/fltk/src/filename_ext.cxx",
            "../contrib/fltk/src/filename_isdir.cxx",
            "../contrib/fltk/src/filename_list.cxx",
            "../contrib/fltk/src/filename_match.cxx",
            "../contrib/fltk/src/filename_setext.cxx",
            "../contrib/fltk/src/fl_arc.cxx",
            "../contrib/fltk/src/fl_arci.cxx",
            "../contrib/fltk/src/fl_ask.cxx",
            "../contrib/fltk/src/fl_boxtype.cxx",
            "../contrib/fltk/src/fl_color.cxx",
            "../contrib/fltk/src/fl_cursor.cxx",
            "../contrib/fltk/src/fl_curve.cxx",
            "../contrib/fltk/src/fl_diamond_box.cxx",
            "../contrib/fltk/src/fl_dnd.cxx",
            "../contrib/fltk/src/fl_draw.cxx",
            "../contrib/fltk/src/fl_draw_image.cxx",
            "../contrib/fltk/src/fl_draw_pixmap.cxx",
            "../contrib/fltk/src/fl_engraved_label.cxx",
            "../contrib/fltk/src/fl_file_dir.cxx",
            "../contrib/fltk/src/fl_font.cxx",
            "../contrib/fltk/src/fl_gleam.cxx",
            "../contrib/fltk/src/fl_gtk.cxx",
            "../contrib/fltk/src/fl_labeltype.cxx",
            "../contrib/fltk/src/fl_line_style.cxx",
            "../contrib/fltk/src/fl_open_uri.cxx",
            "../contrib/fltk/src/fl_oval_box.cxx",
            "../contrib/fltk/src/fl_overlay.cxx",
            "../contrib/fltk/src/fl_overlay_visual.cxx",
            "../contrib/fltk/src/fl_plastic.cxx",
            "../contrib/fltk/src/fl_read_image.cxx",
            "../contrib/fltk/src/fl_rect.cxx",
            "../contrib/fltk/src/fl_round_box.cxx",
            "../contrib/fltk/src/fl_rounded_box.cxx",
            "../contrib/fltk/src/fl_set_font.cxx",
            "../contrib/fltk/src/fl_set_fonts.cxx",
            "../contrib/fltk/src/fl_scroll_area.cxx",
            "../contrib/fltk/src/fl_shadow_box.cxx",
            "../contrib/fltk/src/fl_shortcut.cxx",
            "../contrib/fltk/src/fl_show_colormap.cxx",
            "../contrib/fltk/src/fl_symbols.cxx",
            "../contrib/fltk/src/fl_vertex.cxx",
            "../contrib/fltk/src/ps_image.cxx",
            "../contrib/fltk/src/screen_xywh.cxx",
            "../contrib/fltk/src/fl_utf8.cxx",
            "../contrib/fltk/src/fl_encoding_latin1.cxx",
            "../contrib/fltk/src/fl_encoding_mac_roman.cxx",
            -- .c files
            "../contrib/fltk/src/flstring.c",
            "../contrib/fltk/src/scandir.c",
            "../contrib/fltk/src/numericsort.c",
            "../contrib/fltk/src/vsnprintf.c",
            "../contrib/fltk/src/xutf8/is_right2left.c",
            "../contrib/fltk/src/xutf8/is_spacing.c",
            "../contrib/fltk/src/xutf8/case.c",
            "../contrib/fltk/src/xutf8/utf8Input.c",
            "../contrib/fltk/src/xutf8/utf8Utils.c",
            "../contrib/fltk/src/xutf8/utf8Wrap.c",
            "../contrib/fltk/src/xutf8/keysym2Ucs.c",
            "../contrib/fltk/src/fl_utf.c",
			-- opengl source files
			FLTK_GL_SRC
        },
    }
end