//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
#include "systems/ui/desktop/ve_scroll.h"
#include "systems/ui/desktop/ve_ui_reflector.h"
#include "systems/ui/desktop/ve_justify_input.h"
#include "commands/ve_cmd_add_entity.h"
#include "commands/ve_cmd_update_component.h"
#include "ve_system.h"
#include "FL/Fl_Int_Input.H"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::ComponentView is a widget reflecting a component. It contains a tree
// of reflected parameters that can be modified in real time.
class ComponentView : public Fl_Group
{
public:
	// Callbacks to be called when the component view is being modified.
	struct Callbacks
	{
		ut::Function<void()> on_resize; // Component view changed its size.

		ut::Function<void()> on_update; // Component view has generated a command
		                                // and it must be processed immediately.
	};

	// This structure is used to initialize a component view.
	// It contains a meta snapshot of the managed component and its type.
	struct Proxy
	{
		ut::meta::Snapshot snapshot;
		ut::DynamicType::Handle type;
		Entity::Id entity_id;
	};

	// Constructor. Creates a reflection tree widget.
	//    @param proxy - reference to the intermediate representation of the component.
	//    @param x - horisontal position of the widget in pixels.
	//    @param width - width of the widget in pixels.
	//    @param theme - color theme of the widget.
	//    @param resize_cb - callback to be called after the reflection tree is resized.
	ComponentView(ComponentView::Proxy& proxy,
	              int x,
	              ut::uint32 width,
	              const Theme& theme = Theme(),
	              Callbacks callbacks = Callbacks());

	// Copying, moving is prohibited.
	ComponentView(const ComponentView&) = delete;
	ComponentView(ComponentView&&) = delete;
	ComponentView& operator = (const ComponentView&) = delete;
	ComponentView& operator = (ComponentView&&) = delete;

	// Returns a handle of the dynamic type of the managed component.
	ut::DynamicType::Handle GetType() const;

	// Updates the reflection tree with the new data.
	//    @param proxy - reference to the new representation of the component.
	void Update(ComponentView::Proxy& proxy);

	// Updates size/position of all internal widgets. This function is supposed
	// to be called when one internal widget changes its size and all other widgets
	// must be shifted up or down.
	void UpdateSize();

	// Returns an array of accumulated commands pending to be processed.
	CmdArray FlushCommands();

	// Boolean variable to externally control the relevance of the widget.
	bool is_valid;

private:
	// Helper struct to update component parameters with reflection mechanism.
	struct ReflectionStub : public ut::meta::Reflective
	{
		ReflectionStub(const ut::String& parameter_name,
		               ut::String parameter_value) : name(parameter_name)
		                                           , value(ut::Move(parameter_value)) {};
		void Reflect(ut::meta::Snapshot& s) override { s.Add(value, name); }
		const ut::String& name;
		ut::String value;
	};

	// Helper class to update the value of the desired parameters.
	class CmdModifyItem : public CmdUpdateComponent
	{
	public:
		CmdModifyItem(Entity::Id id,
		              ut::DynamicType::Handle type,
		              ut::JsonDoc json,
		              ut::String name) noexcept;
	private:
		ut::Optional<ut::Error> Modify(ut::meta::Snapshot& parameter);
		ut::JsonDoc serialized_data;
	};

	// Helper class to recreate the value of the desired parameters.
	class CmdRecreateItem : public CmdUpdateComponent
	{
	public:
		CmdRecreateItem(Entity::Id id,
		                ut::DynamicType::Handle component_type,
		                ut::Optional<const ut::DynamicType&> parameter_type,
		                ut::String name) noexcept;
	private:
		ut::Optional<ut::Error> Recreate(ut::meta::Snapshot& parameter);
		ut::Optional<const ut::DynamicType&> type;
	};

	// Creates internal child fltk widget for the caption.
	void CreateCaption(const Theme& theme,
	                   const ut::String& name,
	                   ut::int32 x,
	                   ut::int32 y,
	                   ut::uint32 width);

	// Returns the total expected height of this widget in pixels.
	int CalculateHeight() const;

	// Removes all child widgets from the group.
	void AttachChildWidgets();

	// Adds all child widgets to the group.
	void DetachChildWidgets();

	// Generates a command to delete this component.
	void DeleteThisComponent();

	// Callback to be called when a tree item is modified.
	//    @param parameter_name - name of the modified parameter.
	//    @param data - string representing a modified value.
	ReflectionValue::Callbacks::OnModify OnItemModified;

	// Callback to be called when a tree item is cleared.
	//    @param parameter_name - name of the parameter to clean.
	ReflectionValue::Callbacks::OnClear OnItemCleared;

	// Callback to be called when a tree item is reset to the default value.
	//    @param parameter_name - name of the parameter.
	//    @param dynamic_type - optional dynamic type reference of the
	//                          new object (if parameter is polymorphic).
	ReflectionValue::Callbacks::OnRecreate OnItemRecreated;

	// Callback to be called when a new item is added.
	//    @param parameter_name - name of the array parameter.
	ReflectionValue::Callbacks::OnAddArrItem OnItemAdded;

	// Callback to be called when an item is removed.
	//    @param parameter_name - name of the parameter to be removed.
	ReflectionValue::Callbacks::OnRemoveArrItem OnItemRemoved;

	// Callback that clears the managed container parameter.
	static ut::Optional<ut::Error> ClearItemCallback(ut::meta::Snapshot& parameter);

	// Callback creating a new element in the desired array parameter.
	static ut::Optional<ut::Error> AddNewArrayItemCallback(ut::meta::Snapshot& parameter);

	// Callback removing an element from the desired array parameter.
	static ut::Optional<ut::Error> RemoveArrayItemCallback(ut::meta::Snapshot& parameter);

	// Identifier of the entity holding the ownership of this component.
	Entity::Id entity_id;

	// Type of the managed component.
	ut::DynamicType::Handle type;

	// Text of the caption must be kept in the dedicated stable container,
	// otherwise fltk will be referencing invalid pointer while drawing a caption box.
	ut::UniquePtr<ut::String> cap_text;

	// The group for all caption widgets.
	ut::UniquePtr<Fl_Group> caption;

	// Caption box.
	ut::UniquePtr<Fl_Box> caption_box;

	// Collapse button.
	ut::UniquePtr<BinaryButton> expand_button;

	// Button that deletes this component.
	ut::UniquePtr<Button> delete_component_button;

	// Reflection tree.
	ut::UniquePtr<Reflector> reflector;

	// Commands waiting to be processed. One can take ownership
	// by calling FlushCommands() function.
	ut::Synchronized<CmdArray> pending_commands;

	// Callbacks to be called when the component view is being modified.
	Callbacks callbacks;

	// State of the expand button.
	Button::State expand_state;

	// Height of the caption box in pixels.
	static const int skCapHeight;

	// Margin distance to the left and right borders in pixels.
	static const int skHorizontalOffset;

	// Margin distance to the up and bottom borders in pixels.
	static const int skVerticalOffset;
};

//----------------------------------------------------------------------------//
// ve::ui::EntityView is a widget reflecting an entity. It contains a set of
// widgets reflecting components of the managed entity.
class EntityView : public Fl_Group
{
public:
	// This structure is used to initialize entity views.
	// It contains meta snapshots of all components and the
	// id of the managed entity.
	struct Proxy
	{
		Entity::Id id;
		ut::Array<ComponentView::Proxy> components;
	};

	// Constructor. Creates widgets for all components of the managed entity.
	//    @param proxy - reference to the intermediate representation of the entity.
	//    @param width - width of the widget in pixels.
	//    @param theme - color theme of the widget.
	//    @param resize_cb - callback to be triggered when any component is being resized.
	EntityView(EntityView::Proxy& proxy,
	           ut::uint32 width,
	           const Theme& theme = Theme(),
	           ComponentView::Callbacks component_cb = ComponentView::Callbacks());

	// Returns the id of the managed entity.
	Entity::Id GetId() const;

	// Updates UI representation of the managed entity with the new data.
	//    @param proxy - reference to the new representation of the entity.
	void Update(EntityView::Proxy& proxy);

	// Updates size/position of all internal widgets. This function is supposed
	// to be called when one internal widget changes its size and all other widgets
	// must be shifted up or down.
	void UpdateSize();

	// Returns an array of accumulated commands pending to be processed.
	CmdArray FlushCommands();

	// Applies special effects to this entity indicating that it was newly created.
	//    @param status - applies effects if 'true' and disables effects otherwise.
	void MarkNew(bool status);

	// Boolean variable to externally control the relevance of the widget.
	bool is_valid;

	// Height of the caption box in pixels.
	static const int skCapHeight;

private:
	// Group of controls to add components, delete the entity, etc.
	struct Controls
	{
		ut::UniquePtr<Fl_Group> group;
		ut::UniquePtr<Button> add_component_button;
		ut::UniquePtr<Button> delete_entity_button;
	};

	// Creates internal child fltk widget for the caption.
	void CreateCaption(const Theme& theme,
	                   ut::int32 x,
	                   ut::int32 y,
	                   ut::uint32 width);

	// Creates UI widgets for entity controls (like add component< delete the entity, etc.).
	void InitializeControls(const Theme& theme);

	// Marks all component widgets as 'invalid'
	// ('invalid' means 'not matching any real component in the managed entity').
	void InvalidateComponents();

	// Searches a component by type.
	//    @param component_type - type of the component to be found.
	//    @return - optional reference to the desired widget.
	ut::Optional<ComponentView&> FindComponent(ut::DynamicType::Handle component_type);

	// Adds a new component view to the group.
	//    @param proxy - intermediate representation of the component to add.
	void AddNewComponent(ComponentView::Proxy& proxy);

	// Removes all components that are flagged as 'invalid'
	// ('invalid' means 'not matching any real component in the managed entity').
	void RemoveInvalidComponents();

	// Updates the position of each component view so that they all
	// form a vertical chain.
	void RepositionComponents();

	// Removes all child widgets from the group.
	void AttachChildWidgets();

	// Adds all child widgets to the group.
	void DetachChildWidgets();

	// Updates the caption group of widgets.
	//    @param proxy - reference to the new representation of the entity.
	void UpdateCaption(EntityView::Proxy& proxy);

	// Generates a command to delete this entity.
	void DeleteThisEntity();

	// Shows a dialog box to select a component type, then generates a command
	// to add a new component to the managed entity.
	void CreateNewComponent();

	// Callback triggered when the component is added to the entity.
	void CreateNewComponentCallback(const ut::Optional<ut::Error>& error);

	// Calculates the width of the component view in pixels.
	ut::uint32 CalculateComponentViewWidth() const;

	// Returns the color of the caption box.
	ut::Color<3, ut::byte> GetCaptionColor() const;

	// Returns the color of the interactive elements while hover.
	ut::Color<3, ut::byte> GetHoverColor() const;

	// Id of the managed entity.
	Entity::Id id;

	// Color theme of this widget.
	Theme theme;

	// The group for all caption widgets.
	ut::UniquePtr<Fl_Group> caption;	

	// Text of the caption must be kept in the dedicated stable container,
	// otherwise fltk will be referencing invalid pointer while drawing a caption box.
	ut::UniquePtr<ut::String> cap_text;

	// Caption box widget.
	ut::UniquePtr<Fl_Box> caption_box;

	// The group of widgets to operate with this entity.
	Controls controls;

	// Collapse button.
	ut::UniquePtr<BinaryButton> expand_button;

	// Component views.
	ut::Array< ut::UniquePtr<ComponentView> > components;

	// Callbacks to be triggered when a component is being modified.
	ComponentView::Callbacks component_callbacks;

	// Commands waiting to be processed. One can take ownership
	// by calling FlushCommands() function.
	ut::Synchronized<CmdArray> pending_commands;

	// Indicates if this entity was just created by user.
	bool is_new;

	// State of the expand button.
	Button::State expand_state;
};

//----------------------------------------------------------------------------//
// ve::ui::EntityBrowser is a UI tool to manage entities. One can view and
// modify properties of the desired entity, add new entities and components.
class EntityBrowser : public Window
{
public:
	// Constructor. Initializes base window.
	//    @param x - horisontal position of the window in pixels.
	//    @param y - vertical position of the window in pixels.
	//    @param w - width of the window in pixels.
	//    @param h - height of the window in pixels.
	//    @param theme - color theme of the window.
	EntityBrowser(int x,
	              int y,
	              ut::uint32 w,
	              ut::uint32 h,
	              const Theme& theme = Theme());

	// Shows this window to user and forces the update
	// on the next UpdateEntities() call.
	void show() override;

	// Hides this window. This function is overriden to update
	// 'hidden' status in the thread-safe manner.
	void hide() override;

	// Resizes main window and all entity views.
	void resize(int x, int y, int w, int h) override;

	// Updates UI reflection of the provided entities.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of accumulated commands pending to be processed.
	CmdArray UpdateEntities(ComponentAccessGroup& access);

	// Default metrics of this window in pixels.
	static const ut::uint32 skDefaultWidth;
	static const ut::uint32 skDefaultHeight;

	// Margin distance to the left, right, top and bottom borders in pixels.
	static const int skOffset;

private:
	// The group of controls to add/filter entities
	struct EntityControls
	{
		ut::UniquePtr<Fl_Group> group;
		ut::UniquePtr<Fl_Input> filter_input;
		ut::UniquePtr<Button> add_entity_button;
		ut::Synchronized<ut::String> filter;
	};

	// Intermediate structure containg an information about the
	// entities that must be shown on the desired page.
	struct PageView
	{
		size_t first_filtered_id; // Index of the first filtered entity from
		                          // the @filter_cache in this page.
		size_t entity_count; // The number of entities to be shown.
	};

	// The group of widgets controlling the number of entities
	// showm to the user.
	class PageControls : public Fl_Group
	{
	public:
		PageControls(int x, int y, int w, int h) : Fl_Group(x, y, w, h) {}
		void resize(int px, int py, int mw, int mh) override;

		// Widgets.
		ut::UniquePtr< JustifyInput<Fl_Int_Input> > input;
		ut::UniquePtr<Button> prev_button;
		ut::UniquePtr<Button> next_button;
		ut::UniquePtr<Button> first_button;
		ut::UniquePtr<Button> last_button;
		ut::UniquePtr<Fl_Box> page_count_box;
		ut::UniquePtr<Fl_Box> entity_count_box;

		// Intermediate values.
		ut::Atomic<ut::uint32> page_count;
		ut::Atomic<ut::uint32> entity_count;
		ut::UniquePtr<ut::String> page_count_buffer;
		ut::UniquePtr<ut::String> entity_count_buffer;
		ut::Atomic<ut::uint32> page_id;
		ut::uint32 capacity = 50;
		static constexpr ut::uint32 font_size = 14;
	};

	// Creates UI widgets to add/filter entities.
	void InitializeEntityControls(const Theme& theme);

	// Creates UI widgets controlling the number of entities shown to the user.
	void InitializePageControls(const Theme& theme);

	// Creates an array of EntityView::Proxy from the provided entity map that
	// will be used to update UI component views on the next UI tick.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	void PrepareEntityProxies(ComponentAccessGroup& access);

	// Updates UI representation of all entities.
	void UpdateUi();

	// Searches for the entity view by id.
	//    @param entity_id - identifier of the desired entity.
	//    @return - optional reference to the entity view, or nothing
	//              if not found.
	ut::Optional<EntityView&> FindView(Entity::Id entity_id);

	// Creates a new entity view from the provided proxy.
	void AddEntityView(EntityView::Proxy& proxy);

	// Marks all enitity views as 'invalid'.
	void InvalidateAllViews();

	// Updates entity views with @pending_views proxy array.
	void UpdateViews();

	// Removes all entity views having 'invalid' flag set.
	void RemoveInvalidViews();

	// Updates y-position of all entity view.
	void RepositionViews();

	// Updates size/position of all internal widgets. This function is supposed
	// to be called when one internal widget changes its size and all other widgets
	// must be shifted up or down.
	void UpdateSize();

	// Returns an array of accumulated commands pending to be processed.
	CmdArray FlushCommands();

	// Helper function to preserve scroll position after entity view update.
	void FixScrollPosition();

	// Returns entity view width (same for all views) in pixels.
	int CalculateEntityViewWidth() const;

	// Forces immediate update on the next UpdateEntities() call.
	void ImmediateUpdate();

	// Adds a new entity on the next UpdateEntities() call.
	void AddEntity();

	// Updates entity browser content after an entity was added.
	void AddEntityCallback(const CmdAddEntity::AddResult&);

	// Scrolls view area right to the provided widget.
	void ScrollToWidget(Fl_Widget& widget);

	// Scrolls to the entity view and applies special effects if the
	// user added a new entity.
	void ProcessNewEntity(EntityView& entity_view);

	// Updates @entity_controls.filter string with
	// the value of the input widget.
	void UpdateFilterInput();

	// Updates page id from the input field.
	void UpdatePageId();

	// Updates all page controls.
	void UpdateControls();

	// Filters provided entities and stores filtered
	// indices to the @filter_cache.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - optional index of the filtered value that needs
	//              to be scrolled to.
	ut::Optional<size_t> FilterEntities(ComponentAccess& access);

	// Calculates the first index of the filtered entities and the number of
	// entities to be shown on the current page.
	//    @param filtered_scroll_index - optional index of the filtered value
	//                                   from the @filter_cache array, that must
	//                                   be scrolled to.
	//    @return - the EntityBrowser::PageView structure value.
	PageView PreparePage(const ut::Optional<size_t>& filtered_scroll_index);

	// Contains all entity views located vertically.
	ut::UniquePtr<Scroll> view_area;

	// The group of widgets containing controls to create/filter
	// entities.
	EntityControls entity_controls;

	// The group of widgets controlling the number of entities
	// showm to the user.
	ut::UniquePtr<PageControls> page;

	// Synchronizes update process.
	ut::Mutex mutex;

	// Array of proxies to update entity views on the next UI tick.
	ut::Array<EntityView::Proxy> pending_views;

	// The cache to optimize filtering process.
	ut::Array<Entity::Id> filter_cache;

	// Entity view widgets.
	ut::Array< ut::UniquePtr<EntityView> > entity_views;

	// Timer to count time from the last update.
	ut::time::Counter timer;

	// Indicates if entity browser is active at the moment.
	ut::Synchronized<bool> active;

	// Indicates if all entity views must be updated on the
	// next UpdateEntities() call.
	ut::Synchronized<bool> immediate_update;

	// Commands waiting to be processed on the next UpdateEntities() call.
	ut::Synchronized<CmdArray> pending_commands;

	// Id of the newly created (via the browser) entity. This value is used
	// to properly scroll view area right to this entity view.
	ut::Synchronized< ut::Optional<Entity::Id> > new_entity_id;

	// Height of the control groups in pixels.
	static const ut::uint32 skEntityControlGroupHeight;
	static const ut::uint32 skEntityControlGroupMargin;
	static const ut::uint32 skPageControlGroupHeight;

	// Periods of time (in seconds) between entity updates.
	static const float skUpdatePeriod;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//