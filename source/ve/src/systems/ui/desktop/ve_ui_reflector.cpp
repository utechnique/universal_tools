//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_ui_reflector.h"
#include "systems/ui/desktop/ve_choice_window.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// String to be replaced in xpm color scheme.
const char* Reflector::XpmIcon::skColorLabel = "#change";

// Open node icon xpm template.
const char* Reflector::skOpenXpm[] =
{
	"11 11 2 1",
	".  c None",
	"@  c #change",
	"...@.......",
	"...@@......",
	"...@@@.....",
	"...@@@@....",
	"...@@@@@...",
	"...@@@@@@..",
	"...@@@@@...",
	"...@@@@....",
	"...@@@.....",
	"...@@......",
	"...@......."
};

// Close node icon xpm template.
const char* Reflector::skCloseXpm[] =
{
	"11 11 2 1",
	".  c None",
	"@  c #change",
	"...........",
	"...........",
	"...........",
	"...........",
	"...........",
	"@@@@@@@@@@@",
	".@@.....@@.",
	"..@@...@@..",
	"...@@.@@...",
	"....@@@....",
	".....@....."
};

//----------------------------------------------------------------------------//
// Constructor.
//    @param snapshot - reference to the meta snapshot describing the
//                      current state of the managed parameter.
//    @param cb - reference to the set of value callbacks.
//    @param name - name of the managed parameter.
//    @param left - x position of the widget in pixels.
//    @param width - width of the widget in pixels.
//    @param height - height of the widget in pixels.
//    @param font_size - font size of the text field widget.
ReflectionTextField::ReflectionTextField(ut::meta::Snapshot& snapshot,
                                         const ReflectionValue::Callbacks& cb,
                                         const ut::String& name,
                                         const int left,
                                         const int width,
                                         const int height,
                                         const int font_size) : ReflectionValue(cb, name)
{
	// create input widget
	input = ut::MakeUnique<Fl_Input>(left, 0, width, height);
	input->labelsize(font_size);
	input->textsize(font_size);
	input->show();
	input->when(FL_WHEN_CHANGED);
	input->callback([](Fl_Widget*, void* p) { static_cast<ReflectionTextField*>(p)->OnModify(); }, this);

	// input type
	const ut::String type_name = snapshot.data.parameter->GetTypeName();
	if (type_name == ut::Type<ut::int8>::Name() ||
	    type_name == ut::Type<ut::uint8>::Name() ||
	    type_name == ut::Type<ut::int16>::Name() ||
	    type_name == ut::Type<ut::uint16>::Name() ||
	    type_name == ut::Type<ut::int32>::Name() ||
	    type_name == ut::Type<ut::uint32>::Name() ||
	    type_name == ut::Type<ut::int64>::Name() ||
	    type_name == ut::Type<ut::uint64>::Name())
	{
		input->type(FL_INT_INPUT);
	}
	else if (type_name == ut::Type<float>::Name() ||
	         type_name == ut::Type<double>::Name() ||
	         type_name == ut::Type<long double>::Name())
	{
		input->type(FL_FLOAT_INPUT);
	}

	// update value
	Update(snapshot);
}

// Updates the managed value from the meta snapshot.
void ReflectionTextField::Update(ut::meta::Snapshot& snapshot)
{
	// do not disturb user (if typing)
	if (Fl::focus() == input.Get())
	{
		return;
	}

	const ut::String type = snapshot.data.parameter->GetTypeName();
	const void* data = snapshot.data.parameter->GetAddress();
	
	const ut::String value = CheckType<ut::int8>(type)    ? Print<ut::int8>(data)    :
	                         CheckType<ut::uint8>(type)   ? Print<ut::uint8>(data)   :
	                         CheckType<ut::int16>(type)   ? Print<ut::int16>(data)   :
	                         CheckType<ut::uint16>(type)  ? Print<ut::uint16>(data)  :
	                         CheckType<ut::int32>(type)   ? Print<ut::int32>(data) :
	                         CheckType<ut::uint32>(type)  ? Print<ut::uint32>(data)  :
	                         CheckType<ut::int64>(type)   ? Print<ut::int64>(data)   :
	                         CheckType<ut::uint64>(type)  ? Print<ut::uint64>(data)  :
	                         CheckType<float>(type)       ? Print<float>(data)       :
	                         CheckType<double>(type)      ? Print<double>(data)      :
	                         CheckType<long double>(type) ? Print<long double>(data) :
	                         CheckType<ut::String>(type)  ? Print<ut::String>(data)  :
	                                                        ut::String();

	input->value(value.GetAddress());
}

// Callback to be called every time the managed value is modified in UI.
void ReflectionTextField::OnModify()
{
	callbacks.on_modify(name, input->value());
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param snapshot - reference to the meta snapshot describing the
//                      current state of the managed parameter.
//    @param cb - reference to the set of value callbacks.
//    @param name - name of the managed parameter.
//    @param left - x position of the widget in pixels.
//    @param height - height of the widget in pixels.
ReflectionBool::ReflectionBool(ut::meta::Snapshot& snapshot,
                               const ReflectionValue::Callbacks& cb,
                               const ut::String& name,
                               const int left,
                               const int height) : ReflectionValue(cb, name)
{
	checkbox = ut::MakeUnique<Fl_Check_Button>(left, 0, height, height);
	checkbox->when(FL_WHEN_CHANGED);
	checkbox->callback([](Fl_Widget*, void* p) { static_cast<ReflectionBool*>(p)->OnModify(); }, this);
	Update(snapshot);
}

// Updates the managed value from the meta snapshot.
void ReflectionBool::Update(ut::meta::Snapshot& snapshot)
{
	const bool* data = static_cast<const bool*>(snapshot.data.parameter->GetAddress());
	checkbox->value(*data ? 1 : 0);
}

// Returns the width of this widget in pixels.
int ReflectionBool::GetWidth()
{
	return checkbox->w();
}

// Callback to be called every time the managed value is modified in UI.
void ReflectionBool::OnModify()
{
	// reset focus (to avoid input-frame around the checkbox widget)
	if (Fl::focus() == checkbox.Get() && checkbox->parent() != nullptr)
	{
		Fl::focus(checkbox->parent());
	}

	callbacks.on_modify(name, checkbox->value() ? "true" : "false");
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param x - left position of the button.
//    @param size - width and height of the button in pixels.
//    @param parameter_path - full name of the managed parameter.
//    @param callback - Callback that is triggered when a user
//                      clicks the button.
//    @param theme - color theme.
ClearElementButton::ClearElementButton(int x,
                                       int size,
                                       ut::String parameter_path,
                                       ut::Function<ReflectionValue::Callbacks::OnAddArrItem> cb,
                                       const Theme& theme) : Button(x, 0, size, size)
                                                           , path(ut::Move(parameter_path))
                                                           , callback(ut::Move(cb))
{
	const ut::uint32 icon_size = ut::Max<ut::uint32>(size, 4) - 4;
	SetIcon(ut::MakeShared<Icon>(Icon::CreateTrashBin(icon_size,
	                                                  icon_size,
	                                                  ut::Color<4, ut::byte>(theme.foreground_color.R(),
	                                                                         theme.foreground_color.G(),
	                                                                         theme.foreground_color.B(),
	                                                                         255),
	                                                  icon_size / 16, 1)));

	SetCallback(ut::MemberFunction<ClearElementButton, void()>(this, &ClearElementButton::Clear));
}

// Adds a new element to the managed array parameter.
void ClearElementButton::Clear()
{
	callback(path);
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param x - left position of the button.
//    @param size - width and height of the button in pixels.
//    @param traits - reference to the traits of the managed parameter.
//    @param parameter_path - full name of the managed parameter.
//    @param callback - Callback that is triggered when a user
//                      clicks the button.
//    @param color_theme - color theme.
RecreateElementButton::RecreateElementButton(int x,
                                             int size,
                                             const ut::meta::BaseParameter::Traits& traits,
                                             ut::String parameter_path,
                                             ut::Function<ReflectionValue::Callbacks::OnRecreate> cb,
                                             const Theme& color_theme) : Button(x, 0, size, size)
                                                                       , path(ut::Move(parameter_path))
                                                                       , callback(ut::Move(cb))
                                                                       , theme(color_theme)
{
	const ut::uint32 icon_size = ut::Max<ut::uint32>(size, 4) - 4;
	SetIcon(ut::MakeShared<Icon>(Icon::CreateChange(icon_size,
	                                                icon_size,
	                                                ut::Color<4, ut::byte>(theme.foreground_color.R(),
	                                                                       theme.foreground_color.G(),
	                                                                       theme.foreground_color.B(),
	                                                                       255),
	                                                icon_size / 8)));

	const bool is_container = static_cast<bool>(traits.container);
	if (is_container && traits.container->callbacks.get_factory.IsValid())
	{
		factory = traits.container->callbacks.get_factory();
	}

	SetCallback(ut::MemberFunction<RecreateElementButton, void()>(this, &RecreateElementButton::Recreate));
}

// Resets the managed object to the default copy.
void RecreateElementButton::Recreate()
{
	ut::Optional<const ut::DynamicType&> dynamic_type;
	if (factory)
	{
		// form a list of possible type names
		const size_t type_count = factory->CountTypes();
		ut::Array<ut::String> type_names(type_count);
		for (size_t i = 0; i < type_count; i++)
		{
			const ut::DynamicType& dyn_type = factory->GetTypeByIndex(i);
			type_names[i] = dyn_type.GetName();
		}

		// show a dialog box to choose a desired type
		ut::Vector<2, int> pos = GetFlAbsPosition(this);
		const ut::Optional<ut::uint32> type_id = SelectInDialogWindow(pos.X(),
		                                                              pos.Y(),
		                                                              ChoiceWindow::skDefaultWidth,
		                                                              ChoiceWindow::skDefaultHeight,
		                                                              "Select Type",
		                                                              type_names,
		                                                              theme);
		if (type_id)
		{
			dynamic_type = factory->GetTypeByIndex(type_id.Get());
		}
	}

	callback(path, ut::Move(dynamic_type));

	// fix hover bug
	Fl::awake([](void* ptr) { static_cast<Button*>(ptr)->SetState(Button::state_release); }, this);
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param x - left position of the button.
//    @param size - width and height of the button in pixels.
//    @param parameter_path - full name of the managed parameter.
//    @param callback - Callback that is triggered when a user
//                      clicks the button.
//    @param theme - color theme.
AddNewElementButton::AddNewElementButton(int x,
                                         int size,
                                         ut::String parameter_path,
                                         ut::Function<ReflectionValue::Callbacks::OnAddArrItem> cb,
                                         const Theme& theme) : Button(x, 0, size, size)
                                                             , path(ut::Move(parameter_path))
                                                             , callback(ut::Move(cb))
{
	const ut::uint32 icon_size = ut::Max<ut::uint32>(size, 4) - 4;
	SetIcon(ut::MakeShared<Icon>(Icon::CreatePlus(icon_size,
	                                              icon_size,
	                                              ut::Color<4, ut::byte>(theme.foreground_color.R(),
	                                                                     theme.foreground_color.G(),
	                                                                     theme.foreground_color.B(),
	                                                                     255),
	                                              2, icon_size / 8)));

	SetCallback(ut::MemberFunction<AddNewElementButton, void()>(this, &AddNewElementButton::AddElement));
}

// Adds a new element to the managed array parameter.
void AddNewElementButton::AddElement()
{
	callback(path);
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param x - left position of the button.
//    @param size - width and height of the button in pixels.
//    @param parameter_path - full name of the managed parameter.
//    @param callback - Callback that is triggered when a user
//                      clicks the button.
//    @param theme - color theme.
RemoveElementButton::RemoveElementButton(int x,
                                         int size,
                                         ut::String parameter_path,
                                         ut::Function<ReflectionValue::Callbacks::OnRemoveArrItem> cb,
                                         const Theme& theme) : Button(x, 0, size, size)
                                                             , path(ut::Move(parameter_path))
                                                             , callback(ut::Move(cb))
{
	const ut::uint32 icon_size = ut::Max<ut::uint32>(size, 4) - 4;
	SetIcon(ut::MakeShared<Icon>(Icon::CreateCross(icon_size,
	                                              icon_size,
	                                              ut::Color<4, ut::byte>(theme.foreground_color.R(),
	                                                                     theme.foreground_color.G(),
	                                                                     theme.foreground_color.B(),
	                                                                     255),
	                                              icon_size / 8)));

	SetCallback(ut::MemberFunction<RemoveElementButton, void()>(this, &RemoveElementButton::RemoveElement));
}

// Adds a new element to the managed array parameter.
void RemoveElementButton::RemoveElement()
{
	callback(path);
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param tree - fltk reference to the tree widget.
//    @param tree_item - fltk tree item widget to contain reflection
//                       item.
//    @param path - unique pointer to the string with the full path
//                  to the parameter (tree levels are separated with '/',
//                  for example 'node1/node2/node3').
//    @param snapshot - meta snapshot containing the current state of the
//                      managed parameter.
//    @param cb - reference to the set of value callbacks.
//    @param theme - color theme.
ReflectionTreeItem::ReflectionTreeItem(Fl_Tree& tree,
                                       Fl_Tree_Item& tree_item,
                                       ut::UniquePtr<ut::String> reflection_tree_path,
                                       ut::meta::Snapshot& snapshot,
                                       const ReflectionValue::Callbacks& cb,
                                       const Theme& theme) : item(tree_item)
                                                           , name(ut::MakeUnique<ut::String>(GenerateNodeName(snapshot)))
                                                           , path(ut::Move(reflection_tree_path))
                                                           , is_valid(true)
{
	// measure description text width
	Fl_Font desc_font = 0;
	fl_font(desc_font, skItemTextSize);
	int desc_width, desc_height;
	fl_measure(name->GetAddress(), desc_width, desc_height);

	// start creating internal widgets of the parent tree
	tree.begin();

	// tile widget
	const int attrib_widgets_size = skItemHeight * 4;
	group = ut::MakeUnique<Fl_Group>(0, 0, desc_width + skValueMargin + ReflectionValue::skWidth + attrib_widgets_size, skItemHeight);
	group->callback(ResizeCallback, &tree);
	group->when(FL_WHEN_NOT_CHANGED);

	// description (entity name) widget
	description = ut::MakeUnique<Fl_Box>(0, 0, desc_width, skItemHeight, name->GetAddress());
	description->box(FL_NO_BOX);
	description->align(FL_ALIGN_INSIDE);
	description->labelfont(desc_font);
	description->labelsize(skItemTextSize);
	description->show();

	// calculate metrics for the input widget
	const int input_x = desc_width + skValueMargin;

	// create input widget
	ut::Optional< ut::UniquePtr<ReflectionValue> > input_widget = CreateValueWidget(snapshot,
	                                                                                cb,
	                                                                                path.GetRef(),
	                                                                                input_x,
	                                                                                ReflectionValue::skWidth,
	                                                                                skItemHeight,
	                                                                                skItemTextSize,
	                                                                                theme);
	if (input_widget)
	{
		value = input_widget.Move();
	}

	// create attribute widgets (add element, remove element, reset, etc.)
	attrib_widgets = CreateAttribWidgets(snapshot,
	                                     cb,
	                                     path.GetRef(),
	                                     value.Get() == nullptr ? input_x :
	                                     (input_x + value->GetWidth()),
	                                     skItemHeight,
	                                     skItemTextSize,
	                                     theme);

	// finish creating tile child widgets
	group->end();

	// set tile widget as a displayed widget for the tree item
	item.widget(group.Get());

	// finish creating internal widgets of the parent tree
	tree.end();
}

// Updates this item with the new meta-data.
	//    @param node - reference to the meta snapshot of the new parameter.
void ReflectionTreeItem::Update(ut::meta::Snapshot& node)
{
	// update node name
	ut::String new_name = GenerateNodeName(node);
	if (new_name != name.GetRef())
	{
		name = ut::MakeUnique<ut::String>(GenerateNodeName(node));

		int desc_width, desc_height;
		Fl_Font desc_font = 0;
		fl_font(desc_font, skItemTextSize);
		fl_measure(name->GetAddress(), desc_width, desc_height);

		group->resize(0, 0, desc_width + skValueMargin + ReflectionValue::skWidth, skItemHeight);

		group->remove(description.Get());
		description = ut::MakeUnique<Fl_Box>(0, 0, desc_width, skItemHeight, name->GetAddress());
		description->box(FL_NO_BOX);
		description->align(FL_ALIGN_INSIDE);
		description->labelfont(desc_font);
		description->labelsize(skItemTextSize);
		description->show();
		group->add(description.Get());
	}

	// update value
	if (value)
	{
		value->Update(node);
	}

	is_valid = true;
}

// Returns the user-friendly version of the provided node name.
ut::String ReflectionTreeItem::GenerateNodeName(ut::meta::Snapshot& node)
{
	const ut::String& node_name = node.data.name;
	ut::Optional<ut::meta::Snapshot&> parent = node.GetParent();

	// check if this node is an array element, and if so - return only number
	// without preceding characters
	if (parent)
	{
		const ut::meta::BaseParameter::Traits parameter_traits = parent->data.parameter->GetTraits();
		if (parameter_traits.container && parameter_traits.container->contains_multiple_elements)
		{
			const char* element_name = node_name.GetAddress();
			return element_name + 1;
		}
	}

	// if this node is a unique pointer - return the type of the managed object
	if (parent && parent->data.parameter->GetTypeName() == ut::Type< ut::UniquePtr<void> >::Name())
	{
		ut::BinaryStream stream;
		const ut::meta::Info serialization_info = ut::meta::Info::CreatePure();
		ut::meta::Controller controller(serialization_info);
		controller.SetBinaryOutputStream(stream);

		// serialize unique ptr to the binary string, and the first value to be
		// serialized is a string with the type of the managed object
		parent->data.parameter->Save(controller);
		const ut::Array<ut::byte>& buffer = stream.GetBuffer();
		return reinterpret_cast<const char*>(buffer.GetAddress());
	}

	return node_name;
}

// Callback for resizing tile widget of the tree item. The whole tree and 
// container window must be redrawn while resizing.
//    @param w - tile widget.
//    @param data - expected to be a pointer to Fl_Tree object.
void ReflectionTreeItem::ResizeCallback(Fl_Widget *w, void *data)
{
	Fl_Tree* tree = static_cast<Fl_Tree*>(data);
	tree->redraw();
	w->redraw();
}

// Creates a widget according to the type of the provided meta snapshot of 
// the managed parameter. It can be checkbox for boolean parameter or
// a text field for string parameter or nothing if there is nothing to
// display.
//    @param snapshot - reference to the meta snapshot of the managed
//                      parameter.
//    @param cb - reference to the set of value callbacks.
//    @param name - name of the managed parameter.
//    @param left - x position of the widget in pixels.
//    @param width - width of the widget in pixels.
//    @param height - height of the widget in pixels.
//    @param font_size - font size of the text field widget.
//    @param theme - color theme.
//    @return - optionally a new widget or nothing if nothing to be
//              displayed to user.
ut::Optional< ut::UniquePtr<ReflectionValue> > ReflectionTreeItem::CreateValueWidget(ut::meta::Snapshot& snapshot,
                                                                                     const ReflectionValue::Callbacks& cb,
                                                                                     const ut::String& name,
                                                                                     int left,
                                                                                     int width,
                                                                                     int height,
                                                                                     int font_size,
                                                                                     const Theme& theme)
{
	const ut::String type_name = snapshot.data.parameter->GetTypeName();
	if (type_name == ut::Type<ut::int8>::Name() ||
	    type_name == ut::Type<ut::uint8>::Name() ||
	    type_name == ut::Type<ut::int16>::Name() ||
	    type_name == ut::Type<ut::uint16>::Name() ||
	    type_name == ut::Type<ut::int32>::Name() ||
	    type_name == ut::Type<ut::uint32>::Name() ||
	    type_name == ut::Type<ut::int64>::Name() ||
	    type_name == ut::Type<ut::uint64>::Name() ||
	    type_name == ut::Type<float>::Name() ||
	    type_name == ut::Type<double>::Name() ||
	    type_name == ut::Type<long double>::Name() ||
	    type_name == ut::Type<ut::String>::Name())
	{
        ut::UniquePtr<ReflectionValue> value;
		value = ut::MakeUnique<ReflectionTextField>(snapshot,
		                                            cb,
		                                            name,
		                                            left,
		                                            width,
		                                            height,
		                                            font_size);
		return value;
	}
	else if (type_name == ut::Type<bool>::Name())
	{
        ut::UniquePtr<ReflectionValue> value;
		value = ut::MakeUnique<ReflectionBool>(snapshot,
		                                       cb,
		                                       name,
		                                       left,
		                                       height);
		return value;
	}

	return ut::Optional< ut::UniquePtr<ReflectionValue> >();
}

// Creates attribute widgets. These are special widgets (like buttons) to 
// add element, remove element, reset, etc.
//    @param snapshot - reference to the meta snapshot of the managed
//                      parameter.
//    @param cb - reference to the set of value callbacks.
//    @param name - name of the managed parameter.
//    @param left - x position of the widget in pixels.
//    @param height - height of the widget in pixels.
//    @param font_size - font size of the text field widget.
//    @param theme - color theme.
//    @return - array of widgets.
ut::Array< ut::UniquePtr<Fl_Widget> > ReflectionTreeItem::CreateAttribWidgets(ut::meta::Snapshot& snapshot,
                                                                              const ReflectionValue::Callbacks& cb,
                                                                              const ut::String& name,
                                                                              int left,
                                                                              int height,
                                                                              int font_size,
                                                                              const Theme& theme)
{
	ut::Array< ut::UniquePtr<Fl_Widget> > widgets;

	ut::Optional<ut::meta::Snapshot&> parent = snapshot.GetParent();
	const ut::meta::BaseParameter::Traits traits = snapshot.data.parameter->GetTraits();

	const bool is_container = static_cast<bool>(traits.container);
	const bool can_be_cleared = is_container && traits.container->callbacks.reset.IsValid();
	const bool can_be_changed = is_container && traits.container->callbacks.create.IsValid();
	const bool push_back_allowed = is_container && traits.container->callbacks.push_back.IsValid();
	const bool remove_allowed = parent.HasValue() &&
	                            parent->data.parameter->GetTraits().container.HasValue() &&
	                            parent->data.parameter->GetTraits().container->callbacks.push_back.IsValid();

	if (can_be_cleared)
	{
		widgets.Add(ut::MakeUnique<ClearElementButton>(left,
		                                               height,
		                                               name,
		                                               cb.on_clear,
		                                               theme));
		left += height;
	}

	if (can_be_changed)
	{
		widgets.Add(ut::MakeUnique<RecreateElementButton>(left,
		                                                  height,
		                                                  traits,
		                                                  name,
		                                                  cb.on_recreate,
		                                                  theme));
		left += height;
	}

	if (push_back_allowed)
	{
		widgets.Add(ut::MakeUnique<AddNewElementButton>(left,
		                                                height,
		                                                name,
		                                                cb.on_add_arr_item,
		                                                theme));
		left += height;
	}

	if (remove_allowed)
	{
		widgets.Add(ut::MakeUnique<RemoveElementButton>(left,
		                                                height,
		                                                name,
		                                                cb.on_remove_arr_item,
		                                                theme));
		left += height;
	}

	return widgets;
}

//----------------------------------------------------------------------------->
// Constructor.
//    @param x - left position of the widget in pixels.
//    @param y - upper position of the widget in pixels.
//    @param width - width of the widget in pixels, note that height is
//                   not defined and is variable depending on the total
//                   height of the internal widgets.
//    @param snapshot - reference to the meta snapshot of the managed
//                      parameter.
//    @param theme - color theme of the widget.
Reflector::Reflector(ut::uint32 x,
                     ut::uint32 y,
                     ut::uint32 width,
                     ut::meta::Snapshot& snapshot,
                     const Theme& color_theme) : Fl_Group(x, y, width, 0)
                                               , open_icon(skOpenXpm, ut::Count(skOpenXpm), color_theme.foreground_color)
                                               , close_icon(skCloseXpm, ut::Count(skCloseXpm), color_theme.foreground_color)
                                               , theme(color_theme)
{
	// stop initializing Fl_Group elements
	end();

	// create tree widgets
	tree = ut::MakeUnique<ReflectionTree>(x, y, width, 0); // height is undefined yet
	tree->box(FL_FLAT_BOX);
	tree->color(ConvertToFlColor(theme.background_color));
	tree->item_labelfgcolor(ConvertToFlColor(theme.foreground_color));
	tree->selection_color(ConvertToFlColor(theme.foreground_color));
	tree->selectmode(FL_TREE_SELECT_NONE);
	tree->labeltype(FL_NORMAL_LABEL);
	tree->labelfont(0);
	tree->labelsize(14);
	tree->labelcolor(ConvertToFlColor(theme.foreground_color));
	tree->align(Fl_Align(FL_ALIGN_TOP));
	tree->showroot(0);
	tree->connectorwidth(15);
	tree->connectorcolor(ConvertToFlColor(theme.foreground_color));
	tree->connectorstyle(FL_TREE_CONNECTOR_NONE);
	tree->scrollbar_size(0);
	tree->resizable(nullptr);
	tree->showcollapse(1);
	tree->openicon(open_icon.pixmap.Get());
	tree->closeicon(close_icon.pixmap.Get());
	tree->callback(CollapseCallback, &resize_callback);
	tree->end();
	
	// initialize callbacks
	item_callbacks.on_modify = [&] (const ut::String& name,
	                                const ut::String& value) { item_modified(name, value); };
	item_callbacks.on_clear = [&](const ut::String& name) { item_cleared(name); };
	item_callbacks.on_recreate = [&] (const ut::String& name,
	                                  ut::Optional<const ut::DynamicType&> type) { item_recreated(name, ut::Move(type)); };
	item_callbacks.on_add_arr_item = [&] (const ut::String& name) { item_added(name); };
	item_callbacks.on_remove_arr_item = [&](const ut::String& name) { item_removed(name); };

	// initialize tree
	Update(snapshot);

	// add tree to the group
	add(tree.GetRef());
}

// Constructor. Creates a pixmap from the provided xpm data and
// replaces template color with the provided one.
//    @param xpm_template - pointer to the XPM data.
//    @param xpm_size - number of strings in the XPM.
//    @param color - color to replace template color that is defined
//                   in @skColorLabel constant.
Reflector::XpmIcon::XpmIcon(const char* const* xpm_template,
                            size_t xpm_size,
                            const ut::Color<3, ut::byte>& color)
{
	const ut::String color_label(skColorLabel);

	ut::String color_id;
	color_id.Print("#%x%x%x", color.R(), color.G(), color.B());

	xpm_data.Resize(xpm_size);
	xpm.Resize(xpm_size);

	for (size_t i = 0; i < xpm_size; i++)
	{
		xpm_data[i] = xpm_template[i];
		xpm_data[i].Replace(color_label, color_id);
		xpm[i] = xpm_data[i].GetAddress();
	}

	pixmap = ut::MakeUnique<Fl_Pixmap>(xpm.GetAddress());
}

// Overriden virtual function of the base class (Fl_Group).
void Reflector::resize(int x, int y, int w, int h)
{
	Fl_Group::resize(x, y, w, h);
	UpdateTreeSize();
}

// Updates the tree with new meta snapshot of the managed parameter.
void Reflector::Update(ut::meta::Snapshot& snapshot)
{
	UT_ASSERT(tree);

	InvalidateItems();
	UpdateTreeNode(snapshot, "");
	RemoveInvalidItems();
	UpdateTreeSize();
}

// Updates the height of the group so that it could fit the whole tree
// without showing a vertical scroll bar.
void Reflector::UpdateTreeSize()
{
	const ut::Vector<2, int> tree_size = tree->GetTreeSize();
	int tree_height = tree_size.Y();

	if (tree_size.X() > w())
	{
		tree_height += Fl::scrollbar_size();
	}

	tree->resize(tree->x(),
	             tree->y(),
	             tree->w(),
	             tree_height);

	Fl_Group::resize(this->x(), this->y(), this->w(), tree_height);

}

// Assigns a callback that will be triggered when the tree is resized
// (when an item is collapsed for example)
void Reflector::SetResizeCallback(ut::Function<void()> cb)
{
	resize_callback = ut::Move(cb);
}

// Conects a signal that is triggered when an item is modified.
void Reflector::ConnectModifyItemSignal(ut::Function<ReflectionValue::Callbacks::OnModify> slot)
{
	item_modified.Connect(ut::Move(slot));
}

// Conects a signal that is triggered when an item is cleared.
void Reflector::ConnectClearItemSignal(ut::Function<ReflectionValue::Callbacks::OnClear> slot)
{
	item_cleared.Connect(ut::Move(slot));
}

// Conects a signal that is triggered when an item is recreated.
void Reflector::ConnectRecreateItemSignal(ut::Function<ReflectionValue::Callbacks::OnRecreate> slot)
{
	item_recreated.Connect(ut::Move(slot));
}

// Conects a signal that is triggered when a new item is added.
void Reflector::ConnectAddItemSignal(ut::Function<ReflectionValue::Callbacks::OnAddArrItem> slot)
{
	item_added.Connect(ut::Move(slot));
}

// Conects a signal that is triggered when an item is removed.
void Reflector::ConnectRemoveItemSignal(ut::Function<ReflectionValue::Callbacks::OnRemoveArrItem> slot)
{
	item_removed.Connect(ut::Move(slot));
}

// Recursively updates the provided tree node.
//    @param snapshot - reference to the meta snapshot of the
//                      corresponding parameter.
//    @param root - path to the parent node.
void Reflector::UpdateTreeNode(ut::meta::Snapshot& snapshot, const ut::String& root)
{
	const size_t node_count = snapshot.CountChildren();
	for (size_t i = 0; i < node_count; i++)
	{
		ut::meta::Snapshot& node = snapshot[i];

		ut::UniquePtr<ut::String> path_ptr = ut::MakeUnique<ut::String>(root + "/" + node.data.name);
		const ut::String& path = path_ptr.GetRef();
		
		ut::Optional<size_t> item_widget_id = FindItem(path);
		if (item_widget_id)
		{
			ReflectionTreeItem& item = items[item_widget_id.Get()].GetRef();
			item.Update(node);
		}
		else
		{
			Fl_Tree_Item* tree_item = tree->add(path.GetAddress());
			tree_item->open(); // all items are open by default

			ut::UniquePtr<ReflectionTreeItem> item_widget = ut::MakeUnique<ReflectionTreeItem>(tree.GetRef(),
			                                                                                   *tree_item,
			                                                                                   ut::Move(path_ptr),
			                                                                                   node,
			                                                                                   item_callbacks,
			                                                                                   theme);
			items.Add(ut::Move(item_widget));
		}		

		UpdateTreeNode(node, path);
	}
}

// Searches for the tree node by name.
//    @param path - path to the desired leaf.
//    @return - optionally id of the desired item in the
//              @items array or nothing if not found.
ut::Optional<size_t> Reflector::FindItem(const ut::String& path)
{
	const size_t item_count = items.Count();
	for (size_t i = 0; i < item_count; i++)
	{
		ReflectionTreeItem& item = items[i].GetRef();
		if (item.path.GetRef() == path)
		{
			return i;
		}
	}

	return ut::Optional<size_t>();
}

// Marks all @items as 'invalid'.
void Reflector::InvalidateItems()
{
	const size_t item_count = items.Count();
	for (size_t i = 0; i < item_count; i++)
	{
		ReflectionTreeItem& item = items[i].GetRef();
		item.is_valid = false;
	}
}

// Removes all items marked as 'invalid'.
void Reflector::RemoveInvalidItems()
{
	const size_t item_count = items.Count();
	for (size_t i = item_count; i--; )
	{
		ReflectionTreeItem& item = items[i].GetRef();
		if (item.is_valid)
		{
			continue;
		}
		
		Fl_Tree_Item* fl_tree_item = tree->find_item(item.path->GetAddress());
		if (fl_tree_item)
		{
			tree->remove(fl_tree_item);
		}

		items.Remove(i);
	}
}

// Callback to be called when a tree item is opened or closed.
void Reflector::CollapseCallback(Fl_Widget* widget, void* data)
{
	Fl_Tree* tree = static_cast<Fl_Tree*>(widget);
	if (tree->callback_reason() != FL_TREE_REASON_OPENED &&
		tree->callback_reason() != FL_TREE_REASON_CLOSED)
	{
		return;
	}

	ut::Function<void()>& callback = *static_cast<ut::Function<void()>*>(data);
	if (callback.IsValid())
	{
		callback();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
