//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "systems/ui/desktop/ve_button.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::ReflectionValue is a collection of widgets representing a value of
// the reflected parameter. This value can have different form for different
// parameter types. For examle: strings and integers are represented as a text
// field, boolean values are represented as a checkbox, etc.
class ReflectionValue
{
public:
	// Collection of callbacks triggered on different actions.
	struct Callbacks
	{
		typedef void OnModify(const ut::String& name,
		                      ut::String value);
		typedef void OnAddArrItem(const ut::String& name);
		typedef void OnRemoveArrItem(const ut::String& name);

		ut::Function<OnModify> on_modify; // the value is changed

		ut::Function<OnAddArrItem> on_add_arr_item; // a new element is supposed
		                                            // to be added to the array

		ut::Function<OnRemoveArrItem> on_remove_arr_item; // the array element
		                                                  // is supposed to be
		                                                  // removed.
	};

	// Constructor.
	ReflectionValue(const Callbacks& cb,
	                const ut::String& parameter_name) : callbacks(cb)
	                                                  , name(parameter_name)
	{}

	// Updates a value from the meta snapshot.
	virtual void Update(ut::meta::Snapshot& snapshot) = 0;

	// This class is polymorphic.
	virtual ~ReflectionValue() = default;

protected:
	const Callbacks& callbacks;
	const ut::String& name;
};

//----------------------------------------------------------------------------//
// ve::ui::ReflectionTextField is an UI representation of values that can be
// displayed in a text form (strings, integers, etc).
class ReflectionTextField : public ReflectionValue
{
public:
	// Constructor.
	//    @param snapshot - reference to the meta snapshot describing the
	//                      current state of the managed parameter.
	//    @param cb - reference to the set of value callbacks.
	//    @param name - name of the managed parameter.
	//    @param left - x position of the widget in pixels.
	//    @param width - width of the widget in pixels.
	//    @param height - height of the widget in pixels.
	//    @param font_size - font size of the text field widget.
	ReflectionTextField(ut::meta::Snapshot& snapshot,
	                    const ReflectionValue::Callbacks& cb,
	                    const ut::String& name,
	                    const int left,
	                    const int width,
	                    const int height,
	                    const int font_size);

	// Updates the managed value from the meta snapshot.
	void Update(ut::meta::Snapshot& snapshot) override;

private:
	// Callback to be called every time the managed value is modified in UI.
	void OnModify();

	// Helper function to convert parameter's value to the text form.
	template<typename SimpleType>
	ut::String Print(const void* ptr)
	{
		return ut::Print<SimpleType>(*reinterpret_cast<const SimpleType*>(ptr));
	}

	// Helper function to conveniently check types
	// (shorter than using ut::Type<>::Name()).
	template<typename SimpleType>
	bool CheckType(const ut::String& type_name)
	{
		return type_name == ut::Type<SimpleType>::Name();
	}

	// Input widget.
	ut::UniquePtr<Fl_Input> input;
};

//----------------------------------------------------------------------------//
// ve::ui::ReflectionBool is an UI representation of boolean values.
class ReflectionBool : public ReflectionValue
{
public:
	// Constructor.
	//    @param snapshot - reference to the meta snapshot describing the
	//                      current state of the managed parameter.
	//    @param cb - reference to the set of value callbacks.
	//    @param name - name of the managed parameter.
	//    @param left - x position of the widget in pixels.
	//    @param height - height of the widget in pixels.
	ReflectionBool(ut::meta::Snapshot& snapshot,
	               const ReflectionValue::Callbacks& cb,
	               const ut::String& name,
	               const int left,
	               const int height);

	// Updates the managed value from the meta snapshot.
	void Update(ut::meta::Snapshot& snapshot) override;

private:
	// Callback to be called every time the managed value is modified in UI.
	void OnModify();

	// Checkbox widget.
	ut::UniquePtr<Fl_Check_Button> checkbox;
};

//----------------------------------------------------------------------------//
// ve::ui::ReflectionUniquePtr is an UI representation of the ut::UniquePtr.
class ReflectionUniquePtr : public ReflectionValue
{
public:
	// Constructor.
	//    @param snapshot - reference to the meta snapshot describing the
	//                      current state of the managed parameter.
	//    @param cb - reference to the set of value callbacks.
	//    @param left - x position of the widget in pixels.
	//    @param height - height of the widget in pixels.
	ReflectionUniquePtr(ut::meta::Snapshot& snapshot,
	                    const ReflectionValue::Callbacks& cb,
	                    const int left,
	                    const int height);

	// Updates the managed value from the meta snapshot.
	void Update(ut::meta::Snapshot& snapshot) override;

private:
	// Callback to be called every time the managed value is modified in UI.
	void OnModify();

	// Checkbox widget.
	ut::UniquePtr<Button> button;
};

//----------------------------------------------------------------------------//
// ve::ui::ReflectionTreeItem represents a single node in a reflection tree.
// It has a name and optionally an input widget to operate with the value of
// the managed parameter.
class ReflectionTreeItem
{
public:
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
	ReflectionTreeItem(Fl_Tree& tree,
	                   Fl_Tree_Item& tree_item,
	                   ut::UniquePtr<ut::String> path,
	                   ut::meta::Snapshot& snapshot,
	                   const ReflectionValue::Callbacks& cb);

	// Pointer to the parent tree-item object.
	Fl_Tree_Item& item;
	
	// All internal widgets of this item are gathered in this group.
	ut::UniquePtr<Fl_Group> group;

	// Name of the managed value.
	ut::UniquePtr<ut::String> name;

	// Full path to the managed entity in the reflection tree.
	ut::UniquePtr<ut::String> path;

	// Pointer to the box widget that contains
	// a name of the managed value.
	ut::UniquePtr<Fl_Box> description;

	// Widget for the managed value.
	ut::UniquePtr<ReflectionValue> value;

	// Indicates if this item is still valid.
	bool is_valid;

private:
	// Returns the user-friendly version of the provided node name.
	static ut::String GenerateNodeName(ut::meta::Snapshot& node);

	// Callback for resizing tile widget of the tree item. The whole tree and 
	// container window must be redrawn while resizing.
	//    @param w - tile widget.
	//    @param data - expected to be a pointer to Fl_Tree object.
	static void ResizeCallback(Fl_Widget *w, void *data);

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
	//    @return - optionally a new widget or nothing if nothing to be
	//              displayed to user.
	static ut::Optional< ut::UniquePtr<ReflectionValue> > CreateValueWidget(ut::meta::Snapshot& snapshot,
	                                                                        const ReflectionValue::Callbacks& cb,
	                                                                        const ut::String& name,
	                                                                        int left,
	                                                                        int width,
	                                                                        int height,
	                                                                        int font_size);
};

//----------------------------------------------------------------------------//
// ve::ui::Reflector is a widget to display and operate with the reflected
// representation of the custom entity. To work with this class one must
// provide ut::meta::Snapshot to the constructor (to initialize widgets) and
// call Reflector::Update() periodically to synchronize UI representation with
// the real entity. Connect signals to be able to modify original entity.
class Reflector : public Fl_Group
                , public ut::NonCopyable
{
public:
	// Constructor.
	//    @param x - left position of the widget in pixels.
	//    @param y - upper position of the widget in pixels.
	//    @param width - width of the widget in pixels, note that height is
	//                   not defined and is variable depending on the total
	//                   height of the internal widgets.
	//    @param snapshot - reference to the meta snapshot of the managed
	//                      parameter.
	//    @param theme - color theme of the widget.
	Reflector(ut::uint32 x,
	          ut::uint32 y,
	          ut::uint32 width,
	          ut::meta::Snapshot& snapshot,
	          const Theme& theme = Theme());

	// Overriden virtual function of the base class (Fl_Group).
	void resize(int x, int y, int w, int h) override;

	// Updates the tree with new meta snapshot of the managed parameter.
	void Update(ut::meta::Snapshot& snapshot);

	// Updates the height of the group so that it could fit the whole tree
	// without showing a vertical scroll bar.
	void UpdateTreeSize();

	// Assigns a callback that will be triggered when the tree is resized
	// (when an item is collapsed for example)
	void SetResizeCallback(ut::Function<void()> cb);

	// Conects a signal that is triggered when an item is modified.
	void ConnectModifyItemSignal(ut::Function<ReflectionValue::Callbacks::OnModify> slot);

private:
	// ReflectionTree is a modified Fl_Tree class capable of recalculating 
	// its size without redrawing all items.
	class ReflectionTree : public Fl_Tree
	{
	public:
		ReflectionTree(int x, int y, int w, int h) : Fl_Tree(x, y, w, h)
		{};

		ut::Vector<2, int> GetTreeSize()
		{
			calc_tree();
			return ut::Vector<2, int>(_tree_w, _tree_h);
		}
	};

	// Helper class to conveniently create icons.
	struct XpmIcon
	{
		// Constructor. Creates a pixmap from the provided xpm data and
		// replaces template color with the provided one.
		//    @param xpm_template - pointer to the XPM data.
		//    @param xpm_size - number of strings in the XPM.
		//    @param color - color to replace template color that is defined
		//                   in @skColorLabel constant.
		XpmIcon(const char* const* xpm_template,
		        size_t xpm_size,
		        const ut::Color<3, ut::byte>& color);

		ut::Array<ut::String> xpm_data;
		ut::Array<const char*> xpm;
		ut::UniquePtr<Fl_Pixmap> pixmap;

		// String to be replaced in xpm color scheme.
		static const char* skColorLabel;
	};

	// Recursively updates the provided tree node.
	//    @param snapshot - reference to the meta snapshot of the
	//                      corresponding parameter.
	//    @param root - path to the parent node.
	void UpdateTreeNode(ut::meta::Snapshot& snapshot,
	                    const ut::String& root);

	// Searches for the tree node by name.
	//    @param path - path to the desired leaf.
	//    @return - optionally id of the desired item in the
	//              @items array or nothing if not found.
	ut::Optional<size_t> FindItem(const ut::String& path);

	// Marks all @items as 'invalid'.
	void InvalidateItems();

	// Removes all items marked as 'invalid'.
	void RemoveInvalidItems();

	// Callback to be called when a tree item is opened or closed.
	static void CollapseCallback(Fl_Widget* widget, void* data);

	// Tree widget.
	ut::UniquePtr<ReflectionTree> tree;

	// Tree items (nodes/leaves).
	ut::Array< ut::UniquePtr<ReflectionTreeItem> > items;

	// Set of callbacks triggered when the items are modified.
	ReflectionValue::Callbacks item_callbacks;

	// Callback that to be triggered when the tree is resized
	// (when an item is collapsed for example)
	ut::Function<void()> resize_callback;

	// Signal that is triggered when an item is modified.
	ut::Signal<ReflectionValue::Callbacks::OnModify> item_modified;

	// Icons.
	XpmIcon open_icon;
	XpmIcon close_icon;

	// Open node icon xpm template.
	static const char* skOpenXpm[];

	// Close node icon xpm template.
	static const char* skCloseXpm[];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//