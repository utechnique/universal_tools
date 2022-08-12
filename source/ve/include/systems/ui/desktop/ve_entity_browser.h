//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
#include "systems/ui/desktop/ve_scroll.h"
#include "systems/ui/desktop/ve_ui_reflector.h"
#include "commands/ve_cmd_add_entity.h"
#include "ve_entity_system.h"
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

	// Callback to be called when a tree item is modified.
	//    @param parameter_name - name of the modified parameter.
	//    @param data - string representing a modified value.
	ReflectionValue::Callbacks::OnModify OnItemModified;

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

	// Reflection tree.
	ut::UniquePtr<Reflector> reflector;

	// Commands waiting to be processed. One can take ownership
	// by calling FlushCommands() function.
	ut::Synchronized<CmdArray> pending_commands;

	// Callbacks to be called when the component view is being modified.
	Callbacks callbacks;

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

	// Boolean variable to externally control the relevance of the widget.
	bool is_valid;

	// Height of the caption box in pixels.
	static const int skCapHeight;

private:
	// Creates internal child fltk widget for the caption.
	void CreateCaption(const Theme& theme,
	                   const ut::String& name,
	                   ut::int32 x,
	                   ut::int32 y,
	                   ut::uint32 width);

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

	// Calculates the width of the component view in pixels.
	ut::uint32 CalculateComponentViewWidth() const;

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

	// Collapse button.
	ut::UniquePtr<BinaryButton> expand_button;

	// Component views.
	ut::Array< ut::UniquePtr<ComponentView> > components;

	// Callbacks to be triggered when a component is being modified.
	ComponentView::Callbacks component_callbacks;
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
	//    @param entities - reference to the entity map.
	//    @return - array of accumulated commands pending to be processed.
	CmdArray UpdateEntities(EntitySystem::EntityMap& entities);

	// Default metrics of this window in pixels.
	static const ut::uint32 skDefaultWidth;
	static const ut::uint32 skDefaultHeight;

	// Margin distance to the left, right, top and bottom borders in pixels.
	static const int skOffset;

private:
	// Group of controls to add/filter entities
	struct Controls
	{
		ut::UniquePtr<Fl_Group> group;
		ut::UniquePtr<Fl_Input> filter;
		ut::UniquePtr<Button> add_entity_button;
	};

	// Creates UI widgets to add/filter entities.
	void InitializeControls(const Theme& theme);

	// Creates an array of EntityView::Proxy from the provided entity map that
	// will be used to update UI component views on the next UI tick.
	//    @param entities - reference to the entity map.
	void PrepareEntityProxies(EntitySystem::EntityMap& entities);

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

	// group containing controls to create/filter entities
	Controls controls;

	// Contains all entity views located vertically.
	ut::UniquePtr<Scroll> view_area;

	// Synchronizes update process.
	ut::Mutex mutex;

	// Array of proxies to update entity views on the next UI tick.
	ut::Array<EntityView::Proxy> pending_views;

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

	// Height of the control group in pixels.
	static const ut::uint32 skControlGroupHeight;

	// Height of the browser caption in pixels.
	static const ut::uint32 skCapHeight;

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