//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta_editor.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(commie::MetaEditorWidget, commie::MetaEditorTextField, "meta_editor_input_text")
UT_REGISTER_TYPE(commie::MetaEditorWidget, commie::MetaEditorCheckbox, "meta_editor_input_checkbox")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Callback for resizing tile widget of the tree item. The whole tree and 
// container window must be redrawn while resizing.
//    @param w - tile widget.
//    @param data - expected to be a pointer to Fl_Tree object.
void MetaEditorItemResizeCB(Fl_Widget *w, void *data)
{
	Fl_Tree* tree = static_cast<Fl_Tree*>(data);
	tree->redraw();
	w->redraw();
}

//----------------------------------------------------------------------------//
// default metrics of the meta editor items
const int MetaEditorItem::skItemTextSize = 12;
const int MetaEditorItem::skDescriptionWidth = 100;
const int MetaEditorItem::skItemHeight = 20;
const int MetaEditorItem::skItemRightMargin = 50;

// Default constructor, all fields are set to zero.
MetaEditorItem::MetaEditorItem() : item(nullptr)
                                 , tile(nullptr)
                                 , description(nullptr)
                                 , input(nullptr)
{}

// Copy constructor, @input widget is copied (not moved)
MetaEditorItem::MetaEditorItem(const MetaEditorItem& copy) : item(copy.item)
                                                           , tile(copy.tile)
                                                           , description(copy.description)
{
	// copy input widget only if it exists
	if (copy.input)
	{
		// perform polymorphic copying
		const ut::DynamicType& dynamic_type = copy.input->Identify();
		ut::Polymorphic* clone = dynamic_type.CloneObject(copy.input.GetRef());
		input = ut::UniquePtr<MetaEditorWidget>(static_cast<MetaEditorWidget*>(clone));
	}
}

// Initializes an item, all internal widgets are created here.
//    @param tree - pointer to the parent tree widget.
//    @param tree_item - pointer to the parent tree item widget.
//    @param node - reference to the source text node.
//    @param name - name of entity to be displayed in description
//                  field. The reason because @node.name unfits, is
//                  that fltk doesn't copy strings to internal buffers,
//                  and these strings must be alive all the time.
//    @return - error if failed.
ut::Optional<ut::Error> MetaEditorItem::Create(Fl_Tree* tree,
                                               Fl_Tree_Item* tree_item,
                                               const ut::Tree<ut::text::Node>& node,
                                               const ut::UniquePtr<ut::String>& name)
{
	// set tree item
	item = tree_item;

	// add input widget
	if (node.data.value)
	{
		// start creating internal widgets of the parent tree
		tree->begin();

		// tile widget, note that width is set one pixel bigger than
		// description width (one pixel for input widget). This width will be
		// resized after the first drawing.
		tile = new Fl_Tile(0, 0, skDescriptionWidth + 1, skItemHeight);
		tile->callback(MetaEditorItemResizeCB, tree);
		tile->when(FL_WHEN_NOT_CHANGED);

		// description (entity name) widget
		description = new Fl_Box(0, 0, skDescriptionWidth, skItemHeight, name->GetAddress());
		description->color(FL_WHITE);
		description->box(FL_NO_BOX);
		description->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
		description->labelsize(skItemTextSize);
		description->show();

		// calculate metrics for the input widget
		const int input_x = description->x() + description->w();
		const int input_width = tile->w() - input_x;

		// check if managed type is boolean
		bool is_boolean = false;
		if (node.data.value_type)
		{
			is_boolean = node.data.value_type.Get() == ut::Type<bool>::Name();
		}

		// disable attribute nodes
		bool active = node.data.name != ut::meta::node_names::skType &&
		              node.data.name != ut::meta::node_names::skCount &&
		              node.data.name != ut::meta::node_names::skValueType &&
		              node.data.name != ut::meta::node_names::skId &&
		              node.data.name != ut::meta::node_names::skInfo &&
		              node.data.name != ut::meta::node_names::skVersion &&
		              node.data.name != ut::meta::node_names::skFlags;

		// create input widget of the appropriate type
		if (is_boolean)
		{
			input = ut::MakeUnique<MetaEditorCheckbox>(node.data, input_x, skItemHeight, active);
		}
		else
		{
			input = ut::MakeUnique<MetaEditorTextField>(node.data, input_x, input_width, skItemHeight, skItemTextSize, active);
		}

		// finish creating tile child widgets
		tile->end();

		// set tile widget as a displayed widget for the tree item
		item->widget(tile);

		// finish creating internal widgets of the parent tree
		tree->end();
	}

	// success
	return ut::Optional<ut::Error>();
}

// Resizes all internal widgets to fit new tree size.
//    @param tree - pointer to the parent tree widget.
void MetaEditorItem::Resize(Fl_Tree* tree)
{
	// pointer of the parent tree must be valid
	UT_ASSERT(tree != nullptr);

	// before resizing, check if all widgets exist
	if (tile != nullptr && description != nullptr && input != nullptr)
	{
		// resize tile widget
		ut::int32 tile_width = tree->w() - tile->x();
		tile->resize(tile->x(), tile->y(), tile_width - skItemRightMargin, tile->h());

		// resize input widget
		ut::int32 input_width = tree->w() - description->x() - description->w();
		input->Resize(input_width - skItemRightMargin);
	}
}

// Returns a name of the item, or error if something went wrong.
ut::Result<ut::String, ut::Error> MetaEditorItem::GetName() const
{
	if (item == nullptr)
	{
		return ut::MakeError(ut::error::empty);
	}
	return ut::String(item->label());
}

// Returns a value of the item as a string,
// or error if something went wrong.
ut::Result<ut::String, ut::Error> MetaEditorItem::GetValue() const
{
	if (item == nullptr || !input)
	{
		return ut::MakeError(ut::error::empty);
	}
	return input->GetValue();
}

//----------------------------------------------------------------------------//
// Constructor, sets pointer to the parent window to zero(null).
MetaEditor::MetaEditor() : parent(NULL)
{ }

// Initializes all internal UI widgets.
//    @param parent_window - pointer to the parent (containing) window.
//    @param document - reference to the serialized document to be displayed.
//    @return - error if failed.
ut::Optional<ut::Error> MetaEditor::Create(const Fl_Window* parent_window,
                                           const ut::text::Document& document)
{
	// parent window must be a valid pointer
	if (!parent_window)
	{
		return ut::Error(ut::error::invalid_arg,
		                 "Pointer to the parent window can't be zero (null).");
	}

	// set parent window
	parent = parent_window;

	// create tree widget
	tree = ut::MakeUnique<Fl_Tree>(0, 0, parent->w(), parent->h());
	tree->tooltip("Test tree");
	tree->box(FL_DOWN_BOX);
	tree->color((Fl_Color)55);
	tree->selection_color(FL_SELECTION_COLOR);
	tree->labeltype(FL_NORMAL_LABEL);
	tree->labelfont(0);
	tree->labelsize(14);
	tree->labelcolor(FL_FOREGROUND_COLOR);
	tree->align(Fl_Align(FL_ALIGN_TOP));
	tree->showroot(0);
	tree->connectorwidth(30);
	tree->end();

	// rebuild the UI
	ut::Optional<ut::Error> rebuild_result = Rebuild(document);
	if (!rebuild_result)
	{
		return rebuild_result;
	}

	// success
	return ut::Optional<ut::Error>();
}

// Updates the UI with new document. Old widgets will be deleted.
//    @param document - reference to the new document.
//    @return - error if failed.
ut::Optional<ut::Error> MetaEditor::Rebuild(const ut::text::Document& document)
{
	// parent window must be initialized
	if (!parent)
	{
		return ut::Error(ut::error::empty, "Parent window was not set yet.");
	}

	// tree must be created
	if (!tree)
	{
		return ut::Error(ut::error::empty, "Tree widget was not created yet.");
	}

	// clean resources
	tree->clear();
	name_buffer.Empty();
	items.Empty();

	// create nodes from the document
	for (size_t i = 0; i < document.nodes.GetNum(); i++)
	{
		// root has empty name
		const ut::String root("");

		// recursevly fill node tree
		ut::Result<ut::Optional<ut::Tree<MetaEditorItem> >, ut::Error> add_node_result = AddTreeNode(document.nodes[i], root);
		if (!add_node_result)
		{
			return ut::Error(add_node_result.MoveAlt());
		}

		// skip if new tree is empty
		if (!add_node_result.Get().HasValue())
		{
			continue;
		}

		// add item to the tree
		if (!items.Add(add_node_result.Get().Get()))
		{
			return ut::Error(ut::error::out_of_memory);
		}
	}

	// draw a tree to set tree items positions
	// note that we can't know the real size of the tree widget
	// before actualy drawing it
	tree->draw();

	// resize input control of the items
	ResizeItems();

	// success
	return ut::Optional<ut::Error>();
}

// Updates position and size of the widgets according to the parent window size.
void MetaEditor::ResizeItems()
{
	ut::Tree<MetaEditorItem>::Iterator iterator;
	for (iterator = items.Begin(ut::iterator::first);
	     iterator != items.End(ut::iterator::last);
	     ++iterator)
	{
		iterator->data.Resize(tree.Get());
	}
}

// Serializes all UI data back to the text document. Then an ut::Archive child
// object can be deserialized using this document.
//    @return - error if failed, or ut::XmlDoc otherwise.
ut::Result<ut::XmlDoc, ut::Error> MetaEditor::Save() const
{
	ut::XmlDoc doc;
	for (size_t i = 0; i < items.GetNumChildren(); i++)
	{
		ut::Result<ut::Tree<ut::text::Node>, ut::Error> save_item_result = SaveItem(items[i]);
		if (!save_item_result)
		{
			return ut::MakeError(save_item_result.MoveAlt());
		}
		doc << save_item_result.Get();
	}

	return doc;
}

// Recursively saves meta editor item widget with all it's child widgets.
//    @param item - reference to the item to be saved.
//    @return - error if failed, or ut::Tree<ut::text::Node> object otherwise.
ut::Result<ut::Tree<ut::text::Node>, ut::Error> MetaEditor::SaveItem(const ut::Tree<MetaEditorItem>& item) const
{
	// node to be returned
	ut::Tree<ut::text::Node> node;

	// get item name
	ut::Result<ut::String, ut::Error> get_name_result = item.data.GetName();
	if (!get_name_result)
	{
		return ut::MakeError(get_name_result.MoveAlt());
	}

	// get item value
	ut::Result<ut::String, ut::Error> get_value_result = item.data.GetValue();

	// set node name
	node.data.name = get_name_result.Move();

	// set node value, note that node can have no value
	if (get_value_result)
	{
		node.data.value = get_value_result.Move();
	}

	// add children
	for (size_t i = 0; i < item.GetNumChildren(); i++)
	{
		ut::Result<ut::Tree<ut::text::Node>, ut::Error> save_child_result = SaveItem(item[i]);
		if (!save_child_result)
		{
			return ut::MakeError(save_child_result.MoveAlt());
		}
		node.Add(save_child_result.Move());
	}

	// success
	return node;
}

// Recursively converts a node of a text document to the UI widget form.
// Then this widget will be added to the tree widget.
//    @param node - reference to the text document node to be saved.
//    @param root - reference to the string containg parent folder name.
//    @param item - reference to the parent item.
//    @return - error if function execution fails.
ut::Result<ut::Optional<ut::Tree<MetaEditorItem> >, ut::Error> MetaEditor::AddTreeNode(const ut::Tree<ut::text::Node>& node,
                                                                                       const ut::String& root)
{
	// add node itself
	const ut::String path = root + "/" + node.data.name;
	Fl_Tree_Item* tree_item = tree->add(path.GetAddress());
	if (tree_item == nullptr)
	{
		return ut::MakeError(ut::error::fail);
	}

	// add node name to the buffer
	if (!name_buffer.Add(ut::MakeUnique<ut::String>(node.data.name)))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// create item
	ut::Tree<MetaEditorItem> parent_item;
	ut::Optional<ut::Error> create_item_error = parent_item.data.Create(tree.Get(),
	                                                                    tree_item,
	                                                                    node,
	                                                                    name_buffer.GetLast());
	if (create_item_error)
	{
		return ut::MakeError(create_item_error.Move());
	}

	// add child nodes
	for (size_t child = 0; child < node.GetNumChildren(); child++)
	{
		// create child item
		ut::Result<ut::Optional<ut::Tree<MetaEditorItem> >, ut::Error> add_node_error = AddTreeNode(node[child],
		                                                                                            path.GetAddress());
		if (!add_node_error)
		{
			return ut::MakeError(add_node_error.MoveAlt());
		}

		if (!add_node_error.Get().HasValue())
		{
			continue;
		}

		// add child item to the parent tree
		if (!parent_item.Add(add_node_error.Get().Get()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// success
	return ut::Optional< ut::Tree<MetaEditorItem> >(parent_item);
}

//----------------------------------------------------------------------------//
// Constructor, creates input text field widget, note that @value_type
// of the provided @node can hold type name. If it is integer or float
// then only digits and dot are allowed for input.
//    @param node - reference to the managed text node.
//    @param left - left (x) position of the widget.
//    @param width - width in pixels of the widget.
//    @param height - height in pixels of the widget.
//    @param font_size - size in pixels of the widget font.
MetaEditorTextField::MetaEditorTextField(const ut::text::Node& node,
                                         const int left,
                                         const int width,
                                         const int height,
                                         const int font_size,
                                         const bool active)
{
	// create input widget
	widget = new Fl_Input(left, 0, width, height);
	widget->labelsize(font_size);
	widget->textsize(font_size);
	widget->show();
	widget->value(node.value.Get());

	// deactivate inactive widget
	if (!active)
	{
		widget->deactivate();
	}

	// input type
	if (node.value_type)
	{
		if (node.value_type.Get() == ut::Type<ut::int8>::Name() ||
			node.value_type.Get() == ut::Type<ut::uint8>::Name() ||
			node.value_type.Get() == ut::Type<ut::int16>::Name() ||
			node.value_type.Get() == ut::Type<ut::uint16>::Name() ||
			node.value_type.Get() == ut::Type<ut::int32>::Name() ||
			node.value_type.Get() == ut::Type<ut::uint32>::Name() ||
			node.value_type.Get() == ut::Type<ut::int64>::Name() ||
			node.value_type.Get() == ut::Type<ut::uint64>::Name())
		{
			widget->type(FL_INT_INPUT);
		}
		else if (node.value_type.Get() == ut::Type<float>::Name() ||
			node.value_type.Get() == ut::Type<double>::Name() ||
			node.value_type.Get() == ut::Type<long double>::Name())
		{
			widget->type(FL_FLOAT_INPUT);
		}
	}
}

// Copy constructor, just copies a pointer to the widget.
MetaEditorTextField::MetaEditorTextField(const MetaEditorTextField& copy) : widget(copy.widget)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& MetaEditorTextField::Identify() const
{
	return ut::Identify(this);
}

// Returns the value of the managed entity in a text form.
ut::String MetaEditorTextField::GetValue() const
{
	return ut::String(widget->value());
}

// Resizes contolling widget according to the free space.
//    @param width - free space, available for the widget.
void MetaEditorTextField::Resize(int width) const
{
	widget->resize(widget->x(), widget->y(), width, widget->h());
}

//----------------------------------------------------------------------------//
// Constructor, creates checkbox widget.
//    @param node - reference to the managed text node.
//    @param left - left (x) position of the widget.
//    @param size - width and height in pixels of the widget.
MetaEditorCheckbox::MetaEditorCheckbox(const ut::text::Node& node,
                                       const int left,
                                       const int size,
                                       const bool active)
{
	widget = new Fl_Check_Button(left, 0, size, size);
	if (node.value)
	{
		if (node.value.Get().CompareCaseInsensitive("true") == 0)
		{
			widget->value(1);
		}
	}
}

// Copy constructor, just copies a pointer to the widget.
MetaEditorCheckbox::MetaEditorCheckbox(const MetaEditorCheckbox& copy) : widget(copy.widget)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& MetaEditorCheckbox::Identify() const
{
	return ut::Identify(this);
}

// Returns the value of the managed entity in a text form.
ut::String MetaEditorCheckbox::GetValue() const
{
	return ut::String(widget->value() ? "true" : "false");
}

// Resizes contolling widget according to the free space.
//    @param width - free space, available for the widget.
void MetaEditorCheckbox::Resize(int width) const
{
	// nothing to resize actually.
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//