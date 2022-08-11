//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_entity_browser.h"
#include "commands/ve_cmd_update_component.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Height of the caption box in pixels.
const int ComponentView::skCapHeight = 21;

// Margin distance to the left and right borders in pixels.
const int ComponentView::skHorizontalOffset = 16;

// Margin distance to the up and bottom borders in pixels.
const int ComponentView::skVerticalOffset = 5;

// Height of the caption box in pixels.
const int EntityView::skCapHeight = 25;

// Default width of the entity browser window in pixels.
const ut::uint32 EntityBrowser::skDefaultWidth = 480;

// Default height of the entity browser window in pixels.
const ut::uint32 EntityBrowser::skDefaultHeight = 720;

// Margin distance to the left, right, top and bottom borders in pixels.
const int EntityBrowser::skOffset = 5;

// Height of the control group in pixels.
const ut::uint32 EntityBrowser::skControlGroupHeight = 32;

// Height of the caption in pixels.
const ut::uint32 EntityBrowser::skCapHeight = 24;

// Periods of time (in seconds) between entity updates.
const float EntityBrowser::skUpdatePeriod = 1.0f;

//----------------------------------------------------------------------------//
// Constructor. Creates a reflection tree widget.
//    @param proxy - reference to the intermediate representation of the component.
//    @param x - horisontal position of the widget in pixels.
//    @param width - width of the widget in pixels.
//    @param theme - color theme of the widget.
//    @param resize_cb - callback to be called after the reflection tree is resized.
ComponentView::ComponentView(ComponentView::Proxy& proxy,
                             int x_position,
                             ut::uint32 width,
                             const Theme& theme,
                             Callbacks cb) : Fl_Group(x_position, 0, width, 0)
                                           , is_valid(true)
                                           , entity_id(proxy.entity_id)
                                           , type(proxy.type)
                                           , callbacks(ut::Move(cb))
{
	// stop initializing Fl_Group elements
	end();

	// initialize start position for the child widgets
	x_position += skHorizontalOffset;
	int y_position = skVerticalOffset;

	// create caption group
	CreateCaption(theme,
	              proxy.snapshot.data.name,
	              x_position,
	              y_position,
	              width);
	
	// create reflection tree widget, note that x position exeeds group boundaries,
	// it's a dirty hack to hide reflection tree from user, reflector->hide() here somehow
	// takes too much time
	y_position += caption_box->h() + skVerticalOffset;
	reflector = ut::MakeUnique<Reflector>(x_position - width * 2, y_position, width, proxy.snapshot);

	// connect reflector resize callback
	if (callbacks.on_resize.IsValid())
	{
		reflector->SetResizeCallback(callbacks.on_resize);
	}

	// connect reflector modify callback
	auto on_modify = ut::MemberFunction<ComponentView, ReflectionValue::Callbacks::OnModify>(this, &ComponentView::OnItemModified);
	reflector->ConnectModifyItemSignal(ut::Move(on_modify));

	// resize Fl_Group so that it could fit the reflection tree
	size(w(), CalculateHeight());

	// add all child widgets to the group
	AttachChildWidgets();
}

// Returns a handle of the dynamic type of the managed component.
ut::DynamicType::Handle ComponentView::GetType() const
{
	return type;
}

// Updates the reflection tree with the new data.
//    @param proxy - reference to the new representation of the component.
void ComponentView::Update(ComponentView::Proxy& proxy)
{
	// all child widgets must be removed form the group
	// otherwise they are resized incorrectly
	DetachChildWidgets();

	// update reflection tree
	if (expand_button->IsOn())
	{
		reflector->position(caption->x(), reflector->y());
		reflector->Update(proxy.snapshot);
	}
	else
	{
		// dirty hack to hide reflector
		reflector->position(x() -reflector->w() * 2, reflector->y());
	}

	// the reflection tree has changed its height, so the
	// group must be resized too (to fit reflection tree)
	size(w(), CalculateHeight());

	// attach all child widgets back
	AttachChildWidgets();
}

// Updates size/position of all internal widgets. This function is supposed
// to be called when one internal widget changes its size and all other widgets
// must be shifted up or down.
void ComponentView::UpdateSize()
{
	DetachChildWidgets();
	if (reflector->visible())
	{
		reflector->UpdateTreeSize();
	}
	size(w(), CalculateHeight());
	AttachChildWidgets();
}

// Returns an array of accumulated commands pending to be processed.
CmdArray ComponentView::FlushCommands()
{
	ut::ScopeSyncLock<CmdArray> locked_commands(pending_commands);
	return ut::Move(locked_commands.Get());
}

// Creates internal child fltk widgets.
void ComponentView::CreateCaption(const Theme& theme,
                                  const ut::String& name,
                                  ut::int32 x,
                                  ut::int32 y,
                                  ut::uint32 width)
{
	// create group
	caption = ut::MakeUnique<Fl_Group>(x, y,
	                                   width - skHorizontalOffset,
	                                   skCapHeight);
	const ut::Color<3, ut::byte> cap_color = theme.tab_color.ElementWise() / 2 +
	                                         theme.background_color.ElementWise() / 2;

	// create expand button
	const ut::Color<4, ut::byte> icon_color(theme.foreground_color.R(),
	                                        theme.foreground_color.G(),
	                                        theme.foreground_color.B(),
	                                        255);
	expand_button = ut::MakeUnique<BinaryButton>(caption->x(),
	                                             caption->y(),
	                                             caption->h(),
	                                             caption->h());
	expand_button->SetBackgroundColor(Button::state_release,
	                                  ConvertToFlColor(cap_color));
	expand_button->SetBackgroundColor(Button::state_hover,
	                                  ConvertToFlColor(theme.tab_color));
	expand_button->SetBackgroundColor(Button::state_push,
	                                  ConvertToFlColor(theme.tab_color));
	expand_button->SetOnCallback([&] { callbacks.on_update(); });
	expand_button->SetOffCallback([&] { callbacks.on_update(); });
	expand_button->SetOnIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                   expand_button->h(),
	                                                                   icon_color,
	                                                                   true)));
	expand_button->SetOffIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                    expand_button->h(),
	                                                                    icon_color,
	                                                                    false)));

	// create the background with the text
	cap_text = ut::MakeUnique<ut::String>(name);
	caption_box = ut::MakeUnique<Fl_Box>(caption->x() + expand_button->w(),
	                                     caption->y(),
	                                     caption->w() - expand_button->w(),
	                                     caption->h());
	caption_box->box(FL_FLAT_BOX);
	caption_box->color(ConvertToFlColor(cap_color));
	caption_box->label(cap_text->ToCStr());
	caption_box->show();

	// finish group
	caption->resizable(caption_box.Get());
	caption->end();
}

// Returns the total expected height of this widget in pixels.
int ComponentView::CalculateHeight() const
{
	int reflector_height = expand_button->IsOn() ? (reflector->h() + skVerticalOffset) : 0;
	return caption->h() + reflector_height + skVerticalOffset * 2;
}

// Removes all child widgets from the group.
void ComponentView::AttachChildWidgets()
{
	add(caption.GetRef());
	add(reflector.GetRef());

	// expand button loses focus after detachment from the parent and receives FL_LEAVE 
	// event that forces the button to the 'release' state, below you can see a dirty
	// hack to return the button back to the 'hover' state
	if (expand_button->visible())
	{
		expand_button->hide();
		expand_button->show();
	}
}

// Adds all child widgets to the group.
void ComponentView::DetachChildWidgets()
{
	remove(caption.GetRef());
	remove(reflector.GetRef());
}

// Callback to be called when a tree item is modified.
//    @param parameter_name - name of the modified parameter.
//    @param data - string representing a modified value.
void ComponentView::OnItemModified(const ut::String& full_name,
                                   ut::String value)
{
	// extract the final parameter name from the path
	ut::String parameter_name = full_name;
	const size_t path_len = full_name.Length();
	for (size_t i = path_len; i-- > 0;)
	{
		const char c = full_name[i];
		if (c == '\\' || c == '/')
		{
			parameter_name = full_name.ToCStr() + i + 1;
			break;
		}
	}

	// serialize parameter data
	ReflectionStub dummy(parameter_name, ut::Move(value));
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(dummy, parameter_name, ut::meta::Info::CreatePure());
	ut::JsonDoc json;
	json << snapshot[0];

	// create command
	ut::UniquePtr<Cmd> cmd = ut::MakeUnique<CmdUpdateComponent>(entity_id,
	                                                            type,
	                                                            ut::Move(json),
	                                                            full_name);

	// add command
	ut::ScopeSyncLock<CmdArray> locked_commands(pending_commands);
	CmdArray& commands = locked_commands.Get();
	commands.Add(ut::Move(cmd));

	// inform an owner that the component must be updated
	if (callbacks.on_update.IsValid())
	{
		callbacks.on_update();
	}
}

//----------------------------------------------------------------------------//
// Constructor. Creates widgets for all components of the managed entity.
//    @param proxy - reference to the intermediate representation of the entity.
//    @param width - width of the widget in pixels.
//    @param theme - color theme of the widget.
//    @param resize_cb - callback to be triggered when any component is being resized.
EntityView::EntityView(EntityView::Proxy& proxy,
                       ut::uint32 width,
                       const Theme& ui_theme,
                       ComponentView::Callbacks component_cb) : Fl_Group(0, 0, width, 0)
                                                              , is_valid(true)
                                                              , id(proxy.id)
                                                              , theme(ui_theme)
                                                              , component_callbacks(ut::Move(component_cb))
{
	// stop initializing Fl_Group elements
	end();

	// create caption box
	CreateCaption(theme,
	              ut::Print(id),
	              EntityBrowser::skOffset,
	              EntityBrowser::skOffset,
	              width);
	
	// initialize widgets for all components
	const size_t component_count = proxy.components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		AddNewComponent(proxy.components[i]);
	}
	
	// all components were created with (left,up) position == (0,0)
	// now they must be shifted down to form a vertical chain
	RepositionComponents();
}

// Returns the id of the managed entity.
Entity::Id EntityView::GetId() const
{
	return id;
}

// Updates UI representation of the managed entity with the new data.
//    @param proxy - reference to the new representation of the entity.
void EntityView::Update(EntityView::Proxy& proxy)
{
	// mark all components as 'invalid', so that all components that
	// are present in the current view but are absent in the @proxy
	// could be deleted in the end of this function
	InvalidateComponents();

	// update existing components or add new ones
	const size_t component_count = components.GetNum();
	for (ut::uint32 i = 0; i < component_count; i++)
	{
		ComponentView::Proxy& component_proxy = proxy.components[i];
		ut::Optional<ComponentView&> existing_component = FindComponent(component_proxy.type);
		if (existing_component)
		{
			existing_component->Update(component_proxy);
			existing_component->is_valid = true;
		}
		else
		{
			AddNewComponent(component_proxy);
			add(components.GetLast().GetRef());
		}
	}

	// get rid of all outdated components
	RemoveInvalidComponents();

	// update correct vertical position of the component views
	RepositionComponents();
}

// Updates size/position of all internal widgets. This function is supposed
// to be called when one internal widget changes its size and all other widgets
// must be shifted up or down.
void EntityView::UpdateSize()
{
	const size_t component_count = components.GetNum();
	for (ut::uint32 i = 0; i < component_count; i++)
	{
		components[i]->UpdateSize();
	}

	RepositionComponents();

	redraw();
}

// Returns an array of accumulated commands pending to be processed.
CmdArray EntityView::FlushCommands()
{
	CmdArray cmd;

	const size_t component_count = components.GetNum();
	for (ut::uint32 i = 0; i < component_count; i++)
	{
		cmd += components[i]->FlushCommands();
	}

	return cmd;
}

// Creates internal child fltk widget for the caption.
void EntityView::CreateCaption(const Theme& theme,
                               const ut::String& name,
                               ut::int32 x,
                               ut::int32 y,
                               ut::uint32 width)
{
	// create group
	caption = ut::MakeUnique<Fl_Group>(EntityBrowser::skOffset,
	                                   EntityBrowser::skOffset,
	                                   width - EntityBrowser::skOffset * 2,
	                                   skCapHeight);
	const ut::Color<3, ut::byte> cap_color = theme.tab_color;

	// create expand button
	const ut::Color<4, ut::byte> icon_color(theme.foreground_color.R(),
	                                        theme.foreground_color.G(),
	                                        theme.foreground_color.B(),
	                                        255);
	const ut::Color<3, ut::byte> hover_color = cap_color.ElementWise() / 2 +
	                                           theme.background_color.ElementWise() / 2;
	expand_button = ut::MakeUnique<BinaryButton>(caption->x(),
	                                             caption->y(),
	                                             caption->h(),
	                                             caption->h());
	expand_button->SetBackgroundColor(Button::state_release,
	                                  ConvertToFlColor(cap_color));
	expand_button->SetBackgroundColor(Button::state_hover,
	                                  ConvertToFlColor(hover_color));
	expand_button->SetBackgroundColor(Button::state_push,
	                                  ConvertToFlColor(hover_color));
	expand_button->SetOnCallback([&] { component_callbacks.on_update(); });
	expand_button->SetOffCallback([&] { component_callbacks.on_update(); });
	expand_button->SetOnIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                   expand_button->h(),
	                                                                   icon_color,
	                                                                   true)));
	expand_button->SetOffIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                    expand_button->h(),
	                                                                    icon_color,
	                                                                    false)));

	// create the background with the text
	cap_text = ut::MakeUnique<ut::String>(ut::Print(id));
	caption_box = ut::MakeUnique<Fl_Box>(expand_button->x() + expand_button->w(),
	                                     EntityBrowser::skOffset,
	                                     caption->w() - expand_button->w(),
	                                     skCapHeight);
	caption_box->box(FL_FLAT_BOX);
	caption_box->color(ConvertToFlColor(cap_color));
	caption_box->label(cap_text->ToCStr());
	caption_box->show();

	// finish group
	caption->resizable(caption_box.Get());
	caption->end();
}

// Marks all component widgets as 'invalid'
// ('invalid' means 'not matching any real component in the managed entity')
void EntityView::InvalidateComponents()
{
	const size_t component_count = components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		components[i]->is_valid = false;
	}
}

// Searches a component by type.
//    @param component_type - type of the component to be found.
//    @return - optional reference to the desired widget.
ut::Optional<ComponentView&> EntityView::FindComponent(ut::DynamicType::Handle component_type)
{
	const size_t component_count = components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		ComponentView& component = components[i].GetRef();
		if (component_type == component.GetType())
		{
			return component;
		}
	}

	return ut::Optional<ComponentView&>();
}

// Adds a new component view to the group.
//    @param proxy - intermediate representation of the component to add.
void EntityView::AddNewComponent(ComponentView::Proxy& proxy)
{
	ut::UniquePtr<ComponentView> component_view = ut::MakeUnique<ComponentView>(proxy,
	                                                                            EntityBrowser::skOffset,
	                                                                            CalculateComponentViewWidth(),
	                                                                            theme,
	                                                                            component_callbacks);

	components.Add(ut::Move(component_view));
}

// Removes all components that are flagged as 'invalid'
// ('invalid' means 'not matching any real component in the managed entity')
void EntityView::RemoveInvalidComponents()
{
	const size_t component_count = components.GetNum();
	for (size_t i = component_count; i--; )
	{
		ComponentView& component = components[i].GetRef();

		if (component.is_valid)
		{
			continue;
		}

		component.hide();
		remove(component);

		components.Remove(i);
	}
}

// Updates the position of each component view so that they all
// form a vertical chain.
void EntityView::RepositionComponents()
{
	// all child widgets must be removed form the group
	// otherwise they are resized incorrectly (why??)
	DetachChildWidgets();

	// update vertical position of all components
	const ut::uint32 component_width = CalculateComponentViewWidth();
	int height = y() + caption_box->h() + EntityBrowser::skOffset * 2;
	const size_t component_count = components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		ComponentView& component = components[i].GetRef();
		if (expand_button->IsOn())
		{
			component.resize(0, height, static_cast<int>(component_width), component.h());
			height += component.h();
		}
		else
		{
			// dirty hack to hide component view
			component.position(w() * 2, height);
		}
	}

	// resize group widget so that it could fit all components
	size(w(), height - y());

	// attach all child widgets back
	AttachChildWidgets();
}

// Removes all child widgets from the group.
void EntityView::AttachChildWidgets()
{
	add(caption.GetRef());

	const size_t component_count = components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		add(components[i].GetRef());
	}

	// expand button loses focus after detachment from the parent and receives FL_LEAVE 
	// event that forces the button to the 'release' state, below you can see a dirty
	// hack to return the button back to the 'hover' state
	if (expand_button->visible())
	{
		expand_button->hide();
		expand_button->show();
	}
}

// Adds all child widgets to the group.
void EntityView::DetachChildWidgets()
{
	remove(caption.GetRef());

	const size_t component_count = components.GetNum();
	for (size_t i = 0; i < component_count; i++)
	{
		remove(components[i].GetRef());
	}
}

// Calculates the width of the component view in pixels.
ut::uint32 EntityView::CalculateComponentViewWidth() const
{
	return w() - EntityBrowser::skOffset * 2;
}

//----------------------------------------------------------------------------//
// Constructor. Initializes base window.
//    @param x - horisontal position of the window in pixels.
//    @param y - vertical position of the window in pixels.
//    @param w - width of the window in pixels.
//    @param h - height of the window in pixels.
//    @param theme - color theme of the window.
EntityBrowser::EntityBrowser(int x,
                             int y,
                             ut::uint32 w,
                             ut::uint32 h,
                             const Theme& theme) : Window(x, y, w, h,
                                                          "Entity Browser",
                                                          1, skCapHeight,
                                                          theme,
                                                          Window::has_close_button)
                                                 , immediate_update(true)
                                                 , active(true)
{
	Fl_Double_Window& client_area = GetClientWindow();
	client_area.begin();

	// control group
	InitializeControls(theme);

	// view area
	view_area = ut::MakeUnique<Scroll>(0,
	                                   skControlGroupHeight,
	                                   client_area.w(),
	                                   client_area.h() - skControlGroupHeight);
	view_area->type(Fl_Scroll::VERTICAL);
	view_area->scrollbar_size(Fl::scrollbar_size());
	view_area->resizable(view_area.Get());

	client_area.resizable(view_area.Get());
	client_area.end();
}

// Shows this window to user and forces the update
// on the next UpdateEntities() call.
void EntityBrowser::show()
{
	immediate_update.Set(true);
	Window::show();
	active.Set(true);
}

// Hides this window. This function is overriden to update
// 'hidden' status in the thread-safe manner.
void EntityBrowser::hide()
{
	Window::hide();
	active.Set(false);
}

// Resizes main window and all entity views.
void EntityBrowser::resize(int x, int y, int w, int h)
{
	Window::resize(x, y, w, h);
	UpdateSize();
}

// Updates UI reflection of the provided entities.
//    @param entities - reference to the entity map.
//    @return - array of accumulated commands pending to be processed.
CmdArray EntityBrowser::UpdateEntities(EntitySystem::EntityMap& entities)
{
	ut::ScopeSyncLock<bool> immediate_update_lock(immediate_update);

	// there is no sense to update entities every frame
	const float time_delta = timer.GetTime<ut::time::seconds, float>();
	if (!immediate_update_lock.Get() && (!active.Get() || time_delta < skUpdatePeriod))
	{
		return CmdArray();
	}

	// collect all pending commands from the component views
	CmdArray cmd = FlushCommands();

	// update all components with new data
	if (cmd.IsEmpty())
	{
		PrepareEntityProxies(entities);
		Fl::awake([](void* ptr) { static_cast<EntityBrowser*>(ptr)->UpdateUi(); }, this);
	}

	// reset timer
	timer.Start();

	// reset immediate update flag
	immediate_update_lock.Set(false);

	return cmd;
}

// Creates UI widgets to add/filter entities.
void EntityBrowser::InitializeControls(const Theme& theme)
{
	Fl_Double_Window& client_area = GetClientWindow();

	controls.group = ut::MakeUnique<Fl_Group>(EntityBrowser::skOffset,
	                                          0,
	                                          client_area.w() - EntityBrowser::skOffset * 2,
	                                          skControlGroupHeight);

	controls.filter = ut::MakeUnique<Fl_Input>(controls.group->x(),
	                                           controls.group->y(),
	                                           controls.group->w() - skControlGroupHeight,
	                                           skControlGroupHeight);

	controls.add_entity_button = ut::MakeUnique<Button>(controls.group->x() + controls.filter->w(),
	                                                    controls.group->y(),
	                                                    skControlGroupHeight,
	                                                    skControlGroupHeight);
	controls.add_entity_button->SetIcon(ut::MakeShared<Icon>(Icon::CreatePlus(controls.add_entity_button->w(),
	                                                                          controls.add_entity_button->h(),
	                                                                          ut::Color<4, ut::byte>(0, 200, 0, 255),
	                                                                          4, 4)));
	controls.add_entity_button->SetBackgroundColor(Button::state_release,
	                                               ConvertToFlColor(theme.background_color));
	controls.add_entity_button->SetBackgroundColor(Button::state_hover,
	                                               ConvertToFlColor(theme.tab_color));
	controls.add_entity_button->SetBackgroundColor(Button::state_push,
	                                               ConvertToFlColor(theme.background_color));
	
	controls.group->resizable(controls.filter.Get());
	controls.group->end();
}

// Creates an array of EntityView::Proxy from the provided entity map that
// will be used to update UI component views on the next UI tick.
//    @param entities - reference to the entity map.
void EntityBrowser::PrepareEntityProxies(EntitySystem::EntityMap& entities)
{
	ut::ScopeLock lock(mutex);

	const size_t entity_count = entities.GetNum();

	pending_views.Empty();
	pending_views.Resize(entity_count);

	for (size_t entity_id = 0; entity_id < entity_count; entity_id++)
	{
		ut::Pair<Entity::Id, EntitySystem::ComponentSet>& entity = entities[entity_id];
		const Entity::Id id = entity.first;
		EntitySystem::ComponentSet& components = entity.second;

		EntityView::Proxy& view_proxy = pending_views[entity_id];
		view_proxy.id = id;

		const size_t component_count = components.GetNum();
		for (size_t component_id = 0; component_id < component_count; component_id++)
		{
			const ut::DynamicType& component_type = components[component_id]->Identify();
			ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(components[component_id].Get(),
			                                                          component_type.GetName(),
			                                                          ut::meta::Info::CreateComplete());
			view_proxy.components.Add(ComponentView::Proxy{ ut::Move(snapshot), component_type.GetHandle(), id });
		}
	}
}

// Updates UI representation of all entities.
void EntityBrowser::UpdateUi()
{
	ut::ScopeLock lock(mutex);

	if (!this->visible() || pending_views.IsEmpty())
	{
		return;
	}

	FixScrollPosition();

	InvalidateAllViews();
	
	UpdateViews();

	RepositionViews();
	
	RemoveInvalidViews();

	view_area->redraw();

	pending_views.Empty();
}

// Searches for the entity view by id.
//    @param entity_id - identifier of the desired entity.
//    @return - optional reference to the entity view, or nothing
//              if not found.
ut::Optional<EntityView&> EntityBrowser::FindView(Entity::Id entity_id)
{
	const size_t view_count = entity_views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		ut::UniquePtr<EntityView>& view = entity_views[i];
		if (entity_id == view->GetId())
		{
			return view.GetRef();
		}
	}

	return ut::Optional<EntityView&>();
}

// Creates a new entity view from the provided proxy.
void EntityBrowser::AddEntityView(EntityView::Proxy& proxy)
{
	// initialize component view callbacks
	ComponentView::Callbacks component_callbacks;
	component_callbacks.on_resize = ut::MemberFunction<EntityBrowser, void()>(this, &EntityBrowser::UpdateSize);
	component_callbacks.on_update = ut::MemberFunction<EntityBrowser, void()>(this, &EntityBrowser::ImmediateUpdate);

	// create a new view widget
	view_area->begin();
	ut::UniquePtr<EntityView> new_view = ut::MakeUnique<EntityView>(proxy,
	                                                                CalculateEntityViewWidth(),
	                                                                GetTheme(),
	                                                                ut::Move(component_callbacks));
	view_area->end();

	// add to the parent window
	view_area->add(new_view.GetRef());
	new_view->show();

	// add to the array
	entity_views.Add(ut::Move(new_view));
}

// Marks all enitity views as 'invalid'.
void EntityBrowser::InvalidateAllViews()
{
	const size_t prev_view_count = entity_views.GetNum();
	for (size_t i = 0; i < prev_view_count; i++)
	{
		entity_views[i]->is_valid = false;
	}
}

// Updates entity views with @pending_views proxy array.
void EntityBrowser::UpdateViews()
{
	const ut::uint32 pending_entity_count = static_cast<ut::uint32>(pending_views.GetNum());
	for (ut::uint32 i = 0; i < pending_entity_count; i++)
	{
		EntityView::Proxy& pending_view = pending_views[i];
		ut::Optional<EntityView&> existing_view = FindView(pending_view.id);
		if (existing_view)
		{
			existing_view->Update(pending_view);
			existing_view->is_valid = true;
		}
		else
		{
			AddEntityView(pending_view);
		}
	}
}

// Removes all entity views having 'invalid' flag set.
void EntityBrowser::RemoveInvalidViews()
{
	const size_t view_count = entity_views.GetNum();
	for (size_t i = view_count; i--; )
	{
		EntityView& view = entity_views[i].GetRef();

		if (view.is_valid)
		{
			continue;
		}

		view.hide();
		view_area->remove(view);

		entity_views.Remove(i);
	}
}

// Updates y-position of all entity view.
void EntityBrowser::RepositionViews()
{
	int y = 0;
	const size_t view_count = entity_views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		EntityView& view = entity_views[i].GetRef();
		view.position(0, y);
		y += view.h();
	}
}

// Updates size/position of all internal widgets. This function is supposed
// to be called when one internal widget changes its size and all other widgets
// must be shifted up or down.
void EntityBrowser::UpdateSize()
{
	FixScrollPosition();

	const int view_width = CalculateEntityViewWidth();
	
	const size_t view_count = entity_views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		EntityView& view = entity_views[i].GetRef();
		view.size(view_width, view.h());
		view.UpdateSize();
	}
	
	RepositionViews();

	Fl_Double_Window& client_wnd = GetClientWindow();
	client_wnd.redraw();
}

// Returns an array of accumulated commands pending to be processed.
CmdArray EntityBrowser::FlushCommands()
{
	ut::ScopeLock lock(mutex);

	CmdArray cmd;

	const size_t view_count = entity_views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		cmd += entity_views[i]->FlushCommands();
	}

	return cmd;
}

// Helper function to preserve scroll position after entity view update.
void EntityBrowser::FixScrollPosition()
{
	const int x_scroll = 0;
	const int y_scroll = view_area->yposition();
	view_area->scroll_position = ut::Vector<2, int>(x_scroll, y_scroll);
}

// Returns entity view width (same for all views) in pixels.
int EntityBrowser::CalculateEntityViewWidth() const
{
	UT_ASSERT(view_area.Get());
	return view_area->w() - view_area->scrollbar_size();
}

// Forces immediate update on the next UpdateEntities() call.
void EntityBrowser::ImmediateUpdate()
{
	immediate_update.Set(true);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
