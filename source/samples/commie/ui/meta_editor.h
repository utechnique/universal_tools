//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tile.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// MetaEditorWidget is a parent abstract class for input controls of the items
// for meta editor. It's convenient to use checkboxes for boolean types,
// input text fields for string and so on. You must register derived classes
// using UT_REGISTER_TYPE() macro to fulfill polymorphic behaviour.
class MetaEditorWidget : public ut::Polymorphic
{
public:
	// Identify() method must be implemented for the polymorphic types.
	virtual const ut::DynamicType& Identify() const = 0;

	// Returns the value of the managed entity in a text form.
	virtual ut::String GetValue() const = 0;

	// Resizes contolling widget according to the free space.
	//    @param width - free space, available for the widget.
	virtual void Resize(int width) const = 0;

	// Virtual destructor to fulfill polymorphic behaviour.
	virtual ~MetaEditorWidget() {};
};

//----------------------------------------------------------------------------//
// MetaEditorItem is a container class for the entity to be edited. Unlike the
// MetaEditorWidget class, MetaEditorItem contains a description of the entity,
// tile widget for resizing, tree-item widget, etc.
class MetaEditorItem
{
public:
	// Default constructor, all fields are set to zero.
	MetaEditorItem();

	// Copy constructor, @input widget is copied (not moved)
	MetaEditorItem(const MetaEditorItem& copy);

	// Initializes an item, all internal widgets are created here.
	//    @param tree - pointer to the parent tree widget.
	//    @param tree_item - pointer to the parent tree item widget.
	//    @param node - reference to the source text node.
	//    @param name - name of entity to be displayed in description
	//                  field. The reason because @node.name unfits, is
	//                  that fltk doesn't copy strings to internal buffers,
	//                  and these strings must be alive all the time.
	//    @return - error if failed.
	ut::Optional<ut::Error> Create(Fl_Tree* tree,
	                               Fl_Tree_Item* tree_item,
	                               const ut::Tree<ut::text::Node>& node,
	                               const ut::UniquePtr<ut::String>& name);

	// Resizes all internal widgets to fit new tree size.
	//    @param tree - pointer to the parent tree widget.
	void Resize(Fl_Tree* tree);

	// Returns a name of the item, or error if something went wrong.
	ut::Result<ut::String, ut::Error> GetName() const;

	// Returns a value of the item as a string,
	// or error if something went wrong.
	ut::Result<ut::String, ut::Error> GetValue() const;

private:
	// pointer to the parent tree-item object
	Fl_Tree_Item* item;

	// pointer to the tile widget, this tile is a
	// top-most parent for other internal widgets
	Fl_Tile* tile;

	// pointer to the box widget that contains
	// a name of the managed entity
	Fl_Box* description;

	// pointer to the polymorphic input widget
	ut::UniquePtr<MetaEditorWidget> input;

	// default metrics of the meta editor items
	static const int skItemTextSize;
	static const int skDescriptionWidth;
	static const int skItemHeight;
	static const int skItemRightMargin;
};

//----------------------------------------------------------------------------//
// MetaEditor is a class to edit archives, previously deserialized to the text
// document. MetaEditor needs just a parent fltk window and text document object
// to work. Use MetaEditor::Create() to initialize UI of the editor, and use
// MetaEditor::Save() to serialize changed data back to the text document.
class MetaEditor
{
public:
	// Constructor, sets pointer to the parent window to zero(null).
	MetaEditor();

	// Initializes all internal UI widgets.
	//    @param parent_window - pointer to the parent (containing) window.
	//    @param document - reference to the serialized document to be displayed.
	//    @return - error if failed.
	ut::Optional<ut::Error> Create(const Fl_Window* parent_window,
	                               const ut::text::Document& document);

	// Updates the UI with new document. Old widgets will be deleted.
	//    @param document - reference to the new document.
	//    @return - error if failed.
	ut::Optional<ut::Error> Rebuild(const ut::text::Document& document);

	// Updates position and size of the widgets according to the parent window size.
	void ResizeItems();

	// Serializes all UI data back to the text document. Then an ut::Archive child
	// object can be deserialized using this document.
	//    @return - error if failed, or ut::XmlDoc otherwise.
	ut::Result<ut::XmlDoc, ut::Error> Save() const;

private:
	// Recursively saves meta editor item widget with all it's child widgets.
	//    @param item - reference to the item to be saved.
	//    @return - error if failed, or ut::Tree<ut::text::Node> object otherwise.
	ut::Result<ut::Tree<ut::text::Node>, ut::Error> SaveItem(const ut::Tree<MetaEditorItem>& item) const;

	// Recursively converts a node of a text document to the UI widget form.
	// Then this widget will be added to the tree widget.
	//    @param node - reference to the text document node to be saved.
	//    @param root - reference to the string containg parent folder name.
	//    @param item - reference to the parent item.
	//    @return - error if function execution fails.
	ut::Optional<ut::Error> AddTreeNode(const ut::Tree<ut::text::Node>& node,
	                                    const ut::String& root,
	                                    ut::Tree<MetaEditorItem>& item);

	// pointer to the parent (containing) window
	const Fl_Window* parent;

	// pointer to the tree widget
	ut::UniquePtr<Fl_Tree> tree;

	// tree of items
	ut::Tree<MetaEditorItem> items;

	// array of names for the fltk widgets
	// fltk widgets do not copy strings, so they
	// must be alive all the time
	ut::Array< ut::UniquePtr<ut::String> > name_buffer;
};

//----------------------------------------------------------------------------//
// MetaEditorTextField is a class implementing input widget of the meta editor
// for types that can be represented in text form.
class MetaEditorTextField : public MetaEditorWidget
{
public:
	// Inform UT that this class has no default constructor.
	UT_NO_DEFAULT_CONSTRUCTOR

	// Constructor, creates input text field widget, note that @value_type
	// of the provided @node can hold type name. If it is integer or float
	// then only digits and dot are allowed for input.
	//    @param node - reference to the managed text node.
	//    @param left - left (x) position of the widget.
	//    @param width - width in pixels of the widget.
	//    @param height - height in pixels of the widget.
	//    @param font_size - size in pixels of the widget font.
	MetaEditorTextField(const ut::text::Node& node,
	                    const int left,
	                    const int width,
	                    const int height,
	                    const int font_size);

	// Copy constructor, just copies a pointer to the widget.
	MetaEditorTextField(const MetaEditorTextField& copy);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Returns the value of the managed entity in a text form.
	ut::String GetValue() const;

	// Resizes contolling widget according to the free space.
	//    @param width - free space, available for the widget.
	void Resize(int width) const;
private:
	Fl_Input* widget;
};

//----------------------------------------------------------------------------//
// MetaEditorCheckbox is a class implementing input widget of the meta editor
// for boolean types.
class MetaEditorCheckbox : public MetaEditorWidget
{
public:
	// Inform UT that this class has no default constructor.
	UT_NO_DEFAULT_CONSTRUCTOR

	// Constructor, creates checkbox widget.
	//    @param node - reference to the managed text node.
	//    @param left - left (x) position of the widget.
	//    @param size - width and height in pixels of the widget.
	MetaEditorCheckbox(const ut::text::Node& node,
	                   const int left,
	                   const int size);

	// Copy constructor, just copies a pointer to the widget.
	MetaEditorCheckbox(const MetaEditorCheckbox& copy);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Returns the value of the managed entity in a text form.
	ut::String GetValue() const;

	// Resizes contolling widget according to the free space.
	//    @param width - free space, available for the widget.
	void Resize(int width) const;
private:
	Fl_Check_Button* widget;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
#endif // COMMIE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//