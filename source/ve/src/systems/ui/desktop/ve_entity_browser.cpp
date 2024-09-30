//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_entity_browser.h"
#include "systems/ui/desktop/ve_choice_window.h"
#include "systems/ui/desktop/ve_message_window.h"
#include "commands/ve_cmd_delete_entity.h"
#include "commands/ve_cmd_delete_component.h"
#include "commands/ve_cmd_add_component.h"
#include "components/ve_name_component.h"
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
const int EntityView::skCapHeight = 24;

// Default width of the entity browser window in pixels.
const ut::uint32 EntityBrowser::skDefaultWidth = 480;

// Default height of the entity browser window in pixels.
const ut::uint32 EntityBrowser::skDefaultHeight = 720;

// Margin distance to the left, right, top and bottom borders in pixels.
const int EntityBrowser::skOffset = 5;

// Height of the control groups in pixels.
const ut::uint32 EntityBrowser::EntityControls::skFilterInputHeight = 32;
const ut::uint32 EntityBrowser::EntityControls::skMargin = 4;
const ut::uint32 EntityBrowser::EntityControls::skDelimiterHeight = 20;
const ut::uint32 EntityBrowser::EntityControls::ComponentFilter::skCollapseButtonHeight = 16;
const ut::uint32 EntityBrowser::EntityControls::ComponentFilter::skFilterCheckBoxHeight = 16;
const ut::uint32 EntityBrowser::PageControls::skFontSize = 14;
const ut::uint32 EntityBrowser::PageControls::skHeight = 32;

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
                                           , expand_state(Button::state_release)
{
	// stop initializing Fl_Group elements
	end();

	// initialize start position for the child widgets
	x_position += skHorizontalOffset;
	int y_position = skVerticalOffset / 2;

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

	// connect reflector modify callback
	auto on_clear = ut::MemberFunction<ComponentView, ReflectionValue::Callbacks::OnClear>(this, &ComponentView::OnItemCleared);
	reflector->ConnectClearItemSignal(ut::Move(on_clear));

	// connect reflector recreate callback
	auto on_recreate = ut::MemberFunction<ComponentView, ReflectionValue::Callbacks::OnRecreate>(this, &ComponentView::OnItemRecreated);
	reflector->ConnectRecreateItemSignal(ut::Move(on_recreate));

	// connect reflector add item callback
	auto on_add = ut::MemberFunction<ComponentView, ReflectionValue::Callbacks::OnAddArrItem>(this, &ComponentView::OnItemAdded);
	reflector->ConnectAddItemSignal(ut::Move(on_add));

	// connect reflector add item callback
	auto on_remove = ut::MemberFunction<ComponentView, ReflectionValue::Callbacks::OnRemoveArrItem>(this, &ComponentView::OnItemRemoved);
	reflector->ConnectRemoveItemSignal(ut::Move(on_remove));

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
	const ut::Color<3, ut::byte> cap_color = theme.secondary_tab_color;
	const ut::Color<3, ut::byte> hover_color = cap_color.ElementWise() / 2 +
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
	                                  ConvertToFlColor(hover_color));
	expand_button->SetBackgroundColor(Button::state_push,
	                                  ConvertToFlColor(hover_color));
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

	// 'delete component' button
	delete_component_button = ut::MakeUnique<Button>(caption->x() + caption->w() - skCapHeight,
	                                                 caption->y(),
	                                                 caption->h(),
	                                                 caption->h());
	delete_component_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateCross(delete_component_button->w(),
	                                                                        delete_component_button->h(),
	                                                                        ut::Color<4, ut::byte>(230, 0, 0, 200),
	                                                                        7)));
	delete_component_button->SetBackgroundColor(Button::state_release,
	                                            ConvertToFlColor(cap_color));
	delete_component_button->SetBackgroundColor(Button::state_hover,
	                                            ConvertToFlColor(hover_color));
	delete_component_button->SetBackgroundColor(Button::state_push,
	                                            ConvertToFlColor(hover_color));
	delete_component_button->SetCallback([&] { DeleteThisComponent(); });

	// create the background with the text
	cap_text = ut::MakeUnique<ut::String>(name);
	caption_box = ut::MakeUnique<Fl_Box>(caption->x() + expand_button->w(),
	                                     caption->y(),
	                                     caption->w() - expand_button->w() - delete_component_button->w(),
	                                     caption->h());
	caption_box->box(FL_FLAT_BOX);
	caption_box->color(ConvertToFlColor(cap_color));
	caption_box->label(cap_text->GetAddress());
	caption_box->show();

	// finish group
	caption->resizable(caption_box.Get());
	caption->end();
}

// Returns the total expected height of this widget in pixels.
int ComponentView::CalculateHeight() const
{
	int reflector_height = expand_button->IsOn() ? (reflector->h() + skVerticalOffset) : 0;
	return caption->h() + reflector_height + skVerticalOffset;
}

// Removes all child widgets from the group.
void ComponentView::AttachChildWidgets()
{
	add(caption.GetRef());
	add(reflector.GetRef());

	// expand button loses focus after detachment from the parent and receives FL_LEAVE 
	// event that forces the button to the 'release' state, below you can see a dirty
	// hack to return the button back to the 'hover' state
	expand_button->SetState(expand_state);
}

// Adds all child widgets to the group.
void ComponentView::DetachChildWidgets()
{
	remove(caption.GetRef());
	remove(reflector.GetRef());
	expand_state = expand_button->GetState();
}

// Generates a command to delete this component.
void ComponentView::DeleteThisComponent()
{
	ut::UniquePtr<CmdDeleteComponent> cmd = ut::MakeUnique<CmdDeleteComponent>(entity_id, type);
	cmd->Connect([&](const ut::Optional<ut::Error>&) { callbacks.on_update(); });

	ut::ScopeSyncLock<CmdArray> locked_commands(pending_commands);
	locked_commands.Get().Add(ut::Move(cmd));
	callbacks.on_update();
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
			parameter_name = full_name.GetAddress() + i + 1;
			break;
		}
	}

	// serialize parameter data
	ReflectionStub dummy(parameter_name, ut::Move(value));
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(dummy, parameter_name, ut::meta::Info::CreatePure());
	ut::JsonDoc json;
	json << snapshot[0];

	// create command
	ut::UniquePtr<Cmd> cmd = ut::MakeUnique<CmdModifyItem>(entity_id,
	                                                       type,
	                                                       ut::Move(json),
	                                                       ut::Move(full_name));

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

// Callback to be called when a tree item is cleared.
	//    @param full_name - name of the parameter to clean.
void ComponentView::OnItemCleared(const ut::String& full_name)
{
	ut::UniquePtr<CmdUpdateComponent> cmd = ut::MakeUnique<CmdUpdateComponent>(entity_id,
	                                                                           type,
	                                                                           ClearItemCallback,
	                                                                           ut::Move(full_name));
	cmd->Connect([&](const ut::Optional<ut::Error>&) { callbacks.on_update(); });

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

// Callback to be called when a tree item is reset to the default value.
//    @param full_name - name of the parameter.
//    @param parameter_type - optional dynamic type reference of the
//                            new object (if parameter is polymorphic).
void ComponentView::OnItemRecreated(const ut::String& full_name,
                                    ut::Optional<const ut::DynamicType&> parameter_type)
{
	ut::UniquePtr<CmdRecreateItem> cmd = ut::MakeUnique<CmdRecreateItem>(entity_id,
	                                                                     type,
	                                                                     ut::Move(parameter_type),
	                                                                     ut::Move(full_name));
	cmd->Connect([&](const ut::Optional<ut::Error>&) { callbacks.on_update(); });

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

// Callback to be called when a new item is added.
//    @param full_name - name of the array parameter.
void ComponentView::OnItemAdded(const ut::String& full_name)
{
	ut::UniquePtr<CmdUpdateComponent> cmd = ut::MakeUnique<CmdUpdateComponent>(entity_id,
	                                                                           type,
	                                                                           AddNewArrayItemCallback,
	                                                                           ut::Move(full_name));
	cmd->Connect([&](const ut::Optional<ut::Error>&) { callbacks.on_update(); });

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

// Callback to be called when an item is removed.
//    @param full_name - name of the parameter to be removed.
void ComponentView::OnItemRemoved(const ut::String& full_name)
{
	ut::UniquePtr<CmdUpdateComponent> cmd = ut::MakeUnique<CmdUpdateComponent>(entity_id,
	                                                                           type,
	                                                                           RemoveArrayItemCallback,
	                                                                           ut::Move(full_name));
	cmd->Connect([&](const ut::Optional<ut::Error>&) { callbacks.on_update(); });

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

// Callback that clears the managed container parameter.
ut::Optional<ut::Error> ComponentView::ClearItemCallback(ut::meta::Snapshot& parameter)
{
	const ut::meta::BaseParameter::Traits traits = parameter.data.parameter->GetTraits();

	if (!traits.container.HasValue())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter is not a container.");
	}

	if (!traits.container->callbacks.reset.IsValid())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter does not support reset() callback.");
	}

	traits.container->callbacks.reset();
	return ut::Optional<ut::Error>();
}

// Callback creating a new element in the desired array parameter.
ut::Optional<ut::Error> ComponentView::AddNewArrayItemCallback(ut::meta::Snapshot& parameter)
{
	const ut::meta::BaseParameter::Traits traits = parameter.data.parameter->GetTraits();

	if (!traits.container.HasValue())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter is not a container.");
	}

	if (!traits.container->callbacks.push_back.IsValid())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter does not support push_back() callback.");
	}

	traits.container->callbacks.push_back();
	return ut::Optional<ut::Error>();
}

// Callback removing an element from the desired array parameter.
ut::Optional<ut::Error> ComponentView::RemoveArrayItemCallback(ut::meta::Snapshot& parameter)
{
	ut::Optional<ut::meta::Snapshot&> parent = parameter.GetParent();
	if (!parent)
	{
		return ut::Error(ut::error::fail,
			"Desired parameter cannot be removed, it has no parents.");
	}

	const ut::meta::BaseParameter::Traits traits = parent->data.parameter->GetTraits();

	if (!traits.container.HasValue())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter cannot be removed, its parent is not a container.");
	}

	if (!traits.container->callbacks.remove_element.IsValid())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter does not support remove_element() callback.");
	}

	traits.container->callbacks.remove_element(parameter.data.parameter->GetAddress());
	return ut::Optional<ut::Error>();
}

// Constructor. Creates a command containing a callback that changes the
// desired parameter on execution.
//    @param id - identifier of the desired entity.
//    @param type - handle of of the desired component type.
//    @param json - serialized json data of the new value of the parameter.
//    @param name - name of the desired parameter.
ComponentView::CmdModifyItem::CmdModifyItem(Entity::Id id,
                                            ut::DynamicType::Handle type,
                                            ut::JsonDoc json,
                                            ut::String name) noexcept :
	CmdUpdateComponent(id, type,
                       ut::MemberFunction<CmdModifyItem, CmdUpdateComponent::Callback>(this, &CmdModifyItem::Modify),
                       ut::Move(name)), serialized_data(ut::Move(json))
{}

// Deserializes provided component meta snapshot using preserialized json data.
//     @param parameter - a reference to the meta snapshot
//                        of the desired parameter.
//     @return - optional error if the deserialization process failed.
ut::Optional<ut::Error> ComponentView::CmdModifyItem::Modify(ut::meta::Snapshot& parameter)
{
	try
	{
		serialized_data >> parameter;
	}
	catch (const ut::Error& error)
	{
		return error;
	}

	return ut::Optional<ut::Error>();
}

// Constructor. Creates a command containing a callback that changes the
// desired parameter on execution.
//    @param id - identifier of the desired entity.
//    @param type - handle of of the desired component type.
//    @param json - serialized json data of the new value of the parameter.
//    @param name - name of the desired parameter.
ComponentView::CmdRecreateItem::CmdRecreateItem(Entity::Id id,
                                                ut::DynamicType::Handle component_type,
                                                ut::Optional<const ut::DynamicType&> parameter_type,
                                                ut::String name) noexcept :
	CmdUpdateComponent(id, component_type,
	                   ut::MemberFunction<CmdRecreateItem, CmdUpdateComponent::Callback>(this, &CmdRecreateItem::Recreate),
	                   ut::Move(name)), type(ut::Move(parameter_type))
{}

// Deserializes provided component meta snapshot using preserialized json data.
//     @param parameter - a reference to the meta snapshot
//                        of the desired parameter.
//     @return - optional error if the deserialization process failed.
ut::Optional<ut::Error> ComponentView::CmdRecreateItem::Recreate(ut::meta::Snapshot& parameter)
{
	const ut::meta::BaseParameter::Traits traits = parameter.data.parameter->GetTraits();

	if (!traits.container.HasValue())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter is not a container.");
	}

	if (!traits.container->callbacks.create.IsValid())
	{
		return ut::Error(ut::error::not_supported,
			"Desired parameter does not support create() callback.");
	}

	traits.container->callbacks.create(type);
	return ut::Optional<ut::Error>();
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
                                                              , is_new(false)
                                                              , expand_state(Button::state_release)
{
	// stop initializing Fl_Group elements
	end();

	// create caption box
	CreateCaption(theme, width);
	UpdateCaption(proxy);
	
	// initialize widgets for all components
	const size_t component_count = proxy.components.Count();
	for (size_t i = 0; i < component_count; i++)
	{
		AddNewComponent(proxy.components[i]);
	}
	
	// all components were created with (left,up) position == (0,0)
	// now they must be shifted down to form a vertical chain
	RepositionComponents();
}

// Returns the id of the managed entity.
Entity::Id EntityView::GetEntityId() const
{
	return id;
}

// Updates UI representation of the managed entity with the new data.
//    @param proxy - reference to the new representation of the entity.
void EntityView::Update(EntityView::Proxy& proxy)
{
	// Update the caption widgets (the name-component could have changed)
	UpdateCaption(proxy);

	// mark all components as 'invalid', so that all components that
	// are present in the current view but are absent in the @proxy
	// could be deleted in the end of this function
	InvalidateComponents();

	// update existing components or add new ones
	const size_t component_count = proxy.components.Count();
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
	const size_t component_count = components.Count();
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
	CmdArray cmd = ut::Move(pending_commands.Lock());
	pending_commands.Unlock();

	const size_t component_count = components.Count();
	for (ut::uint32 i = 0; i < component_count; i++)
	{
		cmd += components[i]->FlushCommands();
	}

	return cmd;
}

// Applies special effects to this entity indicating that it was newly created.
//    @param status - applies effects if 'true' and disables effects otherwise.
void EntityView::MarkNew(bool status)
{
	if (is_new == status)
	{
		return;
	}

	is_new = status;

	const Fl_Color bkg_color = ConvertToFlColor(GetCaptionColor());
	const Fl_Color hover_color = ConvertToFlColor(GetHoverColor());

	caption_box->color(bkg_color);

	expand_button->SetBackgroundColor(Button::state_release, bkg_color);
	expand_button->SetBackgroundColor(Button::state_hover, hover_color);
	expand_button->SetBackgroundColor(Button::state_push, hover_color);

	controls.add_component_button->SetBackgroundColor(Button::state_release, bkg_color);
	controls.add_component_button->SetBackgroundColor(Button::state_hover, hover_color);
	controls.add_component_button->SetBackgroundColor(Button::state_push, hover_color);

	controls.delete_entity_button->SetBackgroundColor(Button::state_release, bkg_color);
	controls.delete_entity_button->SetBackgroundColor(Button::state_hover, hover_color);
	controls.delete_entity_button->SetBackgroundColor(Button::state_push, hover_color);

	redraw();
}

// Creates internal child fltk widget for the caption.
void EntityView::CreateCaption(const Theme& theme,
                               ut::uint32 width)
{
	// create group
	caption = ut::MakeUnique<Fl_Group>(EntityBrowser::skOffset,
	                                   0,
	                                   width - EntityBrowser::skOffset * 2,
	                                   skCapHeight);

	// create expand button
	const ut::Color<3, ut::byte> cap_color = GetCaptionColor();
	const ut::Color<3, ut::byte> hover_color = GetHoverColor();
	const ut::Color<4, ut::byte> exp_icon_color(theme.foreground_color.R(),
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
	                                  ConvertToFlColor(hover_color));
	expand_button->SetBackgroundColor(Button::state_push,
	                                  ConvertToFlColor(hover_color));
	expand_button->SetOnCallback([&] { component_callbacks.on_update(); });
	expand_button->SetOffCallback([&] { component_callbacks.on_update(); });
	expand_button->SetOnIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                   expand_button->h(),
	                                                                   exp_icon_color,
	                                                                   true)));
	expand_button->SetOffIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(expand_button->w(),
	                                                                    expand_button->h(),
	                                                                    exp_icon_color,
	                                                                    false)));

	// create the background with the text
	caption_box = ut::MakeUnique<Fl_Box>(caption->x() + caption->h(),
	                                     caption->y(),
	                                     caption->w() - caption->h() - skCapHeight * 2,
	                                     skCapHeight);
	caption_box->box(FL_FLAT_BOX);
	caption_box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	caption_box->color(ConvertToFlColor(GetCaptionColor()));
	caption_box->show();

	// initialize controls to be able to operate with this entity
	InitializeControls(theme);

	// finish group
	caption->resizable(caption_box.Get());
	caption->end();
}

// Creates UI widgets for entity controls (like add component< delete the entity, etc.).
void EntityView::InitializeControls(const Theme& theme)
{
	// create controls group
	controls.group = ut::MakeUnique<Fl_Group>(caption->x() + caption->w() - skCapHeight * 2,
	                                          caption->y(),
	                                          skCapHeight * 2,
	                                          skCapHeight);
	const ut::Color<3, ut::byte> cap_color = GetCaptionColor();
	const ut::Color<3, ut::byte> hover_color = GetHoverColor();

	// 'add component' button
	controls.add_component_button = ut::MakeUnique<Button>(controls.group->x(),
	                                                       controls.group->y(),
	                                                       skCapHeight,
	                                                       skCapHeight);
	controls.add_component_button->SetIcon(ut::MakeShared<Icon>(Icon::CreatePlus(controls.add_component_button->w(),
	                                                                             controls.add_component_button->h(),
	                                                                             ut::Color<4, ut::byte>(0, 200, 0, 180),
	                                                                             2, 7)));
	controls.add_component_button->SetBackgroundColor(Button::state_release,
	                                                  ConvertToFlColor(cap_color));
	controls.add_component_button->SetBackgroundColor(Button::state_hover,
	                                                  ConvertToFlColor(hover_color));
	controls.add_component_button->SetBackgroundColor(Button::state_push,
	                                                  ConvertToFlColor(hover_color));
	controls.add_component_button->SetCallback(ut::MemberFunction<EntityView, void()>(this, &EntityView::CreateNewComponent));

	// 'delete entity' button
	controls.delete_entity_button = ut::MakeUnique<Button>(controls.group->x() + skCapHeight,
	                                                       controls.group->y(),
	                                                       skCapHeight,
	                                                       skCapHeight);
	controls.delete_entity_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateCross(controls.delete_entity_button->w(),
	                                                                              controls.delete_entity_button->h(),
	                                                                              ut::Color<4, ut::byte>(230, 0, 0, 200),
	                                                                              7)));
	controls.delete_entity_button->SetBackgroundColor(Button::state_release,
	                                                  ConvertToFlColor(cap_color));
	controls.delete_entity_button->SetBackgroundColor(Button::state_hover,
	                                                  ConvertToFlColor(hover_color));
	controls.delete_entity_button->SetBackgroundColor(Button::state_push,
	                                                  ConvertToFlColor(hover_color));
	controls.delete_entity_button->SetCallback([&] { DeleteThisEntity(); });

	// finish controls group
	controls.group->resizable(nullptr);
	controls.group->end();
}

// Marks all component widgets as 'invalid'
// ('invalid' means 'not matching any real component in the managed entity')
void EntityView::InvalidateComponents()
{
	const size_t component_count = components.Count();
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
	const size_t component_count = components.Count();
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
	const size_t component_count = components.Count();
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
	int height = y() + caption_box->h() + EntityBrowser::skOffset / 2;
	const size_t component_count = components.Count();
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

	// add offset from the next entity view
	height += EntityBrowser::skOffset / 2;

	// resize group widget so that it could fit all components
	size(w(), height - y());

	// attach all child widgets back
	AttachChildWidgets();
}

// Removes all child widgets from the group.
void EntityView::AttachChildWidgets()
{
	add(caption.GetRef());

	const size_t component_count = components.Count();
	for (size_t i = 0; i < component_count; i++)
	{
		add(components[i].GetRef());
	}

	// buttons lose focus after detachment from the parent and receives FL_LEAVE 
	// event that forces the button to the 'release' state, below you can see a dirty
	// hack to return the button back to the 'hover' state
	expand_button->SetState(expand_state);
}

// Adds all child widgets to the group.
void EntityView::DetachChildWidgets()
{
	remove(caption.GetRef());

	const size_t component_count = components.Count();
	for (size_t i = 0; i < component_count; i++)
	{
		remove(components[i].GetRef());
	}

	expand_state = expand_button->GetState();
}

// Updates the caption group of widgets.
//    @param proxy - reference to the new representation of the entity.
void EntityView::UpdateCaption(EntityView::Proxy& proxy)
{
	ut::String name = ut::String("    #") + ut::Print(id);

	const size_t component_count = proxy.components.Count();
	for (size_t i = 0; i < component_count; i++)
	{
		ComponentView::Proxy& component = proxy.components[i];
		if (component.type == ut::GetPolymorphicHandle<NameComponent>())
		{
			ut::Optional<ut::meta::Snapshot&> name_parameter = component.snapshot.FindChildByName("name");
			if (name_parameter)
			{
				const ut::String* name_ptr = static_cast<const ut::String*>(name_parameter->data.parameter->GetAddress());
				name += ut::String("  ") + *name_ptr;
				break;
			}
		}
	}

	cap_text = ut::MakeUnique<ut::String>(name);
	caption_box->label(cap_text->GetAddress());
}

// Generates a command to delete this entity.
void EntityView::DeleteThisEntity()
{
	ut::UniquePtr<CmdDeleteEntity> cmd = ut::MakeUnique<CmdDeleteEntity>(id);
	cmd->Connect([&](const ut::Optional<ut::Error>&) { component_callbacks.on_update(); });

	ut::ScopeSyncLock<CmdArray> locked_commands(pending_commands);
	locked_commands.Get().Add(ut::Move(cmd));
	component_callbacks.on_update();
}

// Shows a dialog box to select a component type, then generates a command
// to add a new component to the managed entity.
void EntityView::CreateNewComponent()
{
	const ut::uint32 dialog_width = 320;
	const ut::uint32 dialog_height = 480;

	ut::Vector<2, int> caption_position = GetFlAbsPosition(caption.Get());
	caption_position.X() += caption->w() / 2;

	// Create and show the dialog window.
	ut::Optional<const ut::DynamicType&> component_type = SelectDerivedTypeInDialogWindow<Component>(caption_position.X() - dialog_width / 2,
	                                                                                                 caption_position.Y(),
	                                                                                                 dialog_width,
	                                                                                                 dialog_height,
	                                                                                                 "Select Component Type",
	                                                                                                 theme);
	if (component_type)
	{
		ut::UniquePtr<Component> component(static_cast<Component*>(component_type->CreateInstance()));
		ut::UniquePtr<CmdAddComponent> cmd = ut::MakeUnique<CmdAddComponent>(id, ut::Move(component));
		cmd->Connect([&](const ut::Optional<ut::Error>& error) { FltkSync([&]() {CreateNewComponentCallback(error); }); });

		ut::ScopeSyncLock<CmdArray> locked_commands(pending_commands);
		locked_commands.Get().Add(ut::Move(cmd));
		component_callbacks.on_update();
	}

	// fix hover bug
	Fl::awake([](void* ptr) { static_cast<Button*>(ptr)->SetState(Button::state_release); },
	          controls.add_component_button.Get());
}

// Callback triggered when the component is added to the entity.
void EntityView::CreateNewComponentCallback(const ut::Optional<ut::Error>& error)
{
	component_callbacks.on_update();
	controls.add_component_button->SetState(Button::state_release);

	if (!error)
	{
		expand_button->Set(true);
		return;
	}

	const ut::uint32 msg_box_width = ChoiceWindow::skDefaultWidth;
	ut::Vector<2, int> pos = GetFlAbsPosition(caption.Get());
	pos.X() += caption->w() / 2 - msg_box_width / 2;

	ShowMessageWindow(pos.X(),
	                  pos.Y(),
	                  msg_box_width,
	                  ChoiceWindow::skDefaultHeight,
	                  error->GetDesc(),
	                  "Error!",
	                  theme);
}

// Calculates the width of the component view in pixels.
ut::uint32 EntityView::CalculateComponentViewWidth() const
{
	return w() - EntityBrowser::skOffset * 2;
}

// Returns the color of the caption box.
ut::Color<3, ut::byte> EntityView::GetCaptionColor() const
{
	const ut::Color<3, ut::byte> new_effect_color(160, 0, 160);
	const ut::Color<3, ut::byte> normal_color = theme.primary_tab_color;
	const ut::Color<3, ut::byte> new_color = normal_color.ElementWise() / 2 +
	                                         new_effect_color.ElementWise() / 2;
	return is_new ? new_color : normal_color;
}

// Returns the color of the interactive elements while hover.
ut::Color<3, ut::byte> EntityView::GetHoverColor() const
{
	return GetCaptionColor().ElementWise() / 2 +
	       theme.background_color.ElementWise() / 2;
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
                                                          theme)
                                                 , active(true)
                                                 , immediate_update(true)
                                                 , requires_ui_update(false)
{
	// control groups
	InitializeEntityControls(theme);
	InitializePageControls(theme);

	// view area
	const int view_area_vertical_offset = entity_controls.group->h();
	const int view_area_height = this->h() - view_area_vertical_offset - page->h();
	view_area = ut::MakeUnique<Scroll>(0,
	                                   view_area_vertical_offset,
	                                   this->w(),
	                                   view_area_height);
	view_area->type(Fl_Scroll::VERTICAL);
	view_area->scrollbar_size(Fl::scrollbar_size());
	view_area->resizable(view_area.Get());
	view_area->end();

	resizable(view_area.Get());
	size_range(320, 240);
	end();
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
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - array of accumulated commands pending to be processed.
CmdArray EntityBrowser::UpdateEntities(ComponentAccessGroup& group_access)
{
	// reset immediate update flag
	const bool needs_immediate_update = immediate_update.Get();
	immediate_update.Set(false);

	// there is no sense to update entities every frame
	const float time_delta = timer.GetTime<ut::time::seconds, float>();
	if (!needs_immediate_update && (!active.Get() || time_delta < skUpdatePeriod))
	{
		return CmdArray();
	}

	// collect all pending commands from the component views
	CmdArray cmd = FlushCommands();

	// update all components with new data
	if (cmd.IsEmpty())
	{
		MapEntitiesToUiProxies(group_access);

		requires_ui_update.Set(true);
		Fl::awake([](void* ptr) { static_cast<EntityBrowser*>(ptr)->UpdateUi(); }, this);
	}

	// reset timer
	timer.Start();

	return cmd;
}

// Adds or removes the provided component type from filtering.
void EntityBrowser::UpdateComponentFilter(const ut::String& component_name, bool status)
{
	const ut::Result<const ut::DynamicType&, ut::Error> component_type_result = ut::Factory<Component>::GetType(component_name);
	if (!component_type_result)
	{
		return;
	}

	const ut::DynamicType& component_type = component_type_result.Get();

	ut::Array<ut::DynamicType::Handle>& filter = component_type_filter.Lock();

	typedef ut::Array<ut::DynamicType::Handle>::Iterator ComponentFilterIterator;
	const ut::Optional<ComponentFilterIterator> find_result = ut::Find(filter.Begin(),
	                                                                   filter.End(),
	                                                                   component_type.GetHandle());
	if (status && !find_result)
	{
		filter.Add(component_type.GetHandle());
	}
	else if (!status && find_result)
	{
		filter.Remove(find_result.Get());
	}

	const size_t filtered_component_count = filter.Count();

	component_type_filter.Unlock();

	ImmediateUpdate();
}

// Removes all component types from filtering.
void EntityBrowser::ResetComponentFilter()
{
	component_type_filter.Set(ut::Array<ut::DynamicType::Handle>());
	ImmediateUpdate();
}

// Clears entity-id filter.
void EntityBrowser::ResetIdFilter()
{
	entity_id_filter.Set(ut::String(""));
	ImmediateUpdate();
}

// Creates UI widgets to add/filter entities.
void EntityBrowser::InitializeEntityControls(const Theme& theme)
{
	// group
	const int total_height_collapsed = EntityControls::skFilterInputHeight +
	                                   EntityControls::ComponentFilter::skCollapseButtonHeight +
	                                   EntityControls::skDelimiterHeight * 2 +
	                                   EntityControls::skMargin * 5;
	entity_controls.group = ut::MakeUnique<Fl_Group>(EntityBrowser::skOffset,
	                                                 0,
	                                                 w() - EntityBrowser::skOffset,
	                                                 total_height_collapsed);

	// delimiter top
	const ut::Color<3, ut::byte> delimiter_bkg_color(theme.primary_tab_color);
	const ut::Color<3, ut::byte> delimiter_label_color(theme.foreground_color);
	entity_controls.delimiter_top_box = ut::MakeUnique<Fl_Box>(0,
	                                                           entity_controls.group->y() + EntityControls::skMargin,
	                                                           w(),
	                                                           EntityControls::skDelimiterHeight);
	entity_controls.delimiter_top_box->box(FL_BORDER_FRAME);
	entity_controls.delimiter_top_box->color(ConvertToFlColor(delimiter_bkg_color));
	entity_controls.delimiter_top_box->labelcolor(ConvertToFlColor(delimiter_label_color));
	entity_controls.delimiter_top_box->label("Filter");
	entity_controls.delimiter_top_box->show();

	// filter input
	entity_controls.filter_input = ut::MakeUnique<Fl_Input>(entity_controls.group->x(),
	                                                        entity_controls.delimiter_top_box->y() +
	                                                        entity_controls.delimiter_top_box->h() +
	                                                        EntityControls::skMargin,
	                                                        entity_controls.group->w() - EntityControls::skFilterInputHeight * 2,
	                                                        EntityControls::skFilterInputHeight);
	entity_controls.filter_input->when(FL_WHEN_CHANGED);
	entity_controls.filter_input->callback([](Fl_Widget*, void* p) { static_cast<EntityBrowser*>(p)->UpdateFilterInput(); }, this);
	entity_controls.filter_input->color(ConvertToFlColor(theme.background_color.ElementWise() / 2 + theme.background_color.ElementWise() / 4));

	// add entity button
	entity_controls.add_entity_button = ut::MakeUnique<Button>(entity_controls.group->x() + entity_controls.filter_input->w(),
	                                                           entity_controls.filter_input->y(),
	                                                           EntityControls::skFilterInputHeight,
	                                                           EntityControls::skFilterInputHeight);
	entity_controls.add_entity_button->SetIcon(ut::MakeShared<Icon>(Icon::CreatePlus(entity_controls.add_entity_button->w(),
	                                                                                 entity_controls.add_entity_button->h(),
	                                                                                 ut::Color<4, ut::byte>(0, 200, 0, 255),
	                                                                                 4, 4)));
	entity_controls.add_entity_button->SetBackgroundColor(Button::state_release,
	                                                      ConvertToFlColor(theme.background_color));
	entity_controls.add_entity_button->SetBackgroundColor(Button::state_hover,
	                                                      ConvertToFlColor(theme.button_hover_color));
	entity_controls.add_entity_button->SetBackgroundColor(Button::state_push,
	                                                      ConvertToFlColor(theme.button_push_color));
	entity_controls.add_entity_button->SetCallback([&] { AddEntity(); });

	// clear filter button
	entity_controls.clear_filter_button = ut::MakeUnique<Button>(entity_controls.group->x() +
	                                                             entity_controls.filter_input->w() +
	                                                             EntityControls::skFilterInputHeight,
	                                                             entity_controls.filter_input->y(),
	                                                             EntityControls::skFilterInputHeight,
	                                                             EntityControls::skFilterInputHeight);
	entity_controls.clear_filter_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateCross(entity_controls.clear_filter_button->w(),
	                                                                                    entity_controls.clear_filter_button->h(),
	                                                                                    ut::Color<4, ut::byte>(230, 230, 230, 200),
	                                                                                    6)));
	entity_controls.clear_filter_button->SetBackgroundColor(Button::state_release,
	                                                        ConvertToFlColor(theme.background_color));
	entity_controls.clear_filter_button->SetBackgroundColor(Button::state_hover,
	                                                        ConvertToFlColor(theme.button_hover_color));
	entity_controls.clear_filter_button->SetBackgroundColor(Button::state_push,
	                                                        ConvertToFlColor(theme.button_push_color));
	entity_controls.clear_filter_button->SetCallback([&] { ClearFilter(); });
	
	// component filter expand
	const ut::Color<4, ut::byte> icon_color(theme.foreground_color.R(),
	                                        theme.foreground_color.G(),
	                                        theme.foreground_color.B(),
	                                        255);
	entity_controls.components.expand_button = ut::MakeUnique<BinaryButton>(entity_controls.group->x(),
	                                                                        entity_controls.filter_input->y() +
	                                                                        entity_controls.filter_input->h() +
	                                                                        EntityControls::skMargin,
	                                                                        w() - EntityBrowser::skOffset * 2,
	                                                                        EntityControls::ComponentFilter::skCollapseButtonHeight);
	entity_controls.components.expand_button->SetOnCallback([&] { UpdateSize(); Fl::focus(entity_controls.filter_input.Get()); });
	entity_controls.components.expand_button->SetOffCallback([&] { UpdateSize(); Fl::focus(entity_controls.filter_input.Get()); });
	entity_controls.components.expand_button->SetOnIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(entity_controls.components.expand_button->h(),
	                                                                                              entity_controls.components.expand_button->h(),
	                                                                                              icon_color,
	                                                                                              true)));
	entity_controls.components.expand_button->SetOffIcon(ut::MakeShared<Icon>(Icon::CreateCollapse(entity_controls.components.expand_button->h(),
	                                                                                               entity_controls.components.expand_button->h(),
	                                                                                               icon_color,
	                                                                                               false)));
	entity_controls.components.expand_empty_bkg_color = theme.secondary_tab_color;
	entity_controls.components.expand_bkg_color = entity_controls.components.expand_empty_bkg_color + ut::Color<3, ut::byte>(0, 20, 0);
	entity_controls.components.expand_bkg_color.R() -= 20;
	entity_controls.components.expand_bkg_color.B() -= 20;
	entity_controls.components.expand_button->SetBackgroundColor(Button::state_release,
	                                                             ConvertToFlColor(entity_controls.components.expand_empty_bkg_color));
	entity_controls.components.expand_button->SetBackgroundColor(Button::state_hover,
	                                                             ConvertToFlColor(entity_controls.components.expand_empty_bkg_color));
	entity_controls.components.expand_button->SetBackgroundColor(Button::state_push,
	                                                             ConvertToFlColor(theme.button_push_color));

	// component filter description
	entity_controls.components.counter_box = ut::MakeUnique<Fl_Box>(entity_controls.components.expand_button->x(),
	                                                               entity_controls.components.expand_button->y(),
	                                                               entity_controls.components.expand_button->w(),
	                                                               entity_controls.components.expand_button->h());
	entity_controls.components.counter_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	entity_controls.components.counter_box->labelcolor(ConvertToFlColor(theme.caption_icon_color));

	// component checkboxes
	const int component_count = static_cast<int>(ut::Factory<Component>::CountTypes());
	const int component_height = EntityControls::ComponentFilter::skFilterCheckBoxHeight;
	const int component_group_height = component_height * component_count;
	entity_controls.components.group = ut::MakeUnique<Fl_Group>(entity_controls.group->x(),
	                                                            entity_controls.components.expand_button->y() +
	                                                            entity_controls.components.expand_button->h() +
	                                                            EntityControls::skMargin,
	                                                            entity_controls.group->w(),
	                                                            component_group_height);

	for (int component_iterator = 0; component_iterator < component_count; component_iterator++)
	{
		ut::UniquePtr<Fl_Check_Button> component_checkbox = ut::MakeUnique<Fl_Check_Button>(entity_controls.components.group->x(),
		                                                                                    entity_controls.components.group->y() + component_iterator * component_height,
		                                                                                    entity_controls.components.group->w(),
		                                                                                    component_height);

		const ut::DynamicType& dynamic_type = ut::Factory<Component>::GetTypeByIndex(component_iterator);
		component_checkbox->label(dynamic_type.GetName().GetAddress());
		component_checkbox->when(FL_WHEN_CHANGED);
		component_checkbox->callback([](Fl_Widget* w, void* p) { EntityBrowser* b = static_cast<EntityBrowser*>(p);
		                                                         Fl_Check_Button* cb = static_cast<Fl_Check_Button*>(w);
		                                                         b->UpdateComponentFilter(ut::String(cb->label()), cb->value() ? true : false);
		                                                         Fl::focus(b->entity_controls.filter_input.Get());
		                                                         b->ImmediateUpdate(); },
		                             this);

		entity_controls.components.checkboxes.Add(ut::Move(component_checkbox));
	}

	entity_controls.components.group->end();
	entity_controls.components.group->hide();

	// delimiter bottom
	entity_controls.delimiter_bottom_box = ut::MakeUnique<Fl_Box>(0,
	                                                              entity_controls.components.expand_button->y() +
	                                                              entity_controls.components.expand_button->h() +
	                                                              EntityControls::skMargin,
	                                                              w(),
	                                                              EntityControls::skDelimiterHeight);
	entity_controls.delimiter_bottom_box->box(FL_BORDER_FRAME);
	entity_controls.delimiter_bottom_box->color(ConvertToFlColor(delimiter_bkg_color));
	entity_controls.delimiter_bottom_box->color2(ConvertToFlColor(delimiter_bkg_color));
	entity_controls.delimiter_bottom_box->labelcolor(ConvertToFlColor(delimiter_label_color));
	entity_controls.delimiter_bottom_box->label("Entities");
	entity_controls.delimiter_bottom_box->show();

	entity_controls.group->resizable(nullptr);
	entity_controls.group->end();
}

// Creates UI widgets controlling the number of entities shown to the user.
void EntityBrowser::InitializePageControls(const Theme& theme)
{
	const int icon_margin = 3;
	const ut::Color<4, ut::byte> icon_color(theme.foreground_color.R(),
	                                        theme.foreground_color.G(),
	                                        theme.foreground_color.B(),
	                                        180);
	const int input_offset = 4;
	int input_width, input_height;

	// calculate metrics of the central input widget
	Fl_Font input_font = 0;
	fl_font(input_font, PageControls::skFontSize);
	fl_measure("0000", input_width, input_height);
	input_width += input_offset;
	input_height += input_offset;

	// create the widget group
	page = ut::MakeUnique<PageControls>(EntityBrowser::skOffset,
	                                    h() - PageControls::skHeight,
	                                    w() - EntityBrowser::skOffset * 2,
	                                    PageControls::skHeight);

    // set default page id
    page->page_id.Store(1);

	// create central input widget
	page->input = ut::MakeUnique< JustifyInput<Fl_Int_Input> >(page->x() +
	                                                           page->w() / 2 - input_width / 2,
	                                                           page->y() +
	                                                           page->h() / 2 - input_height / 2,
	                                                           input_width,
	                                                           input_height);
	page->input->labelfont(input_font);
	page->input->labelsize(PageControls::skFontSize);
	page->input->align(FL_ALIGN_CENTER);
	page->input->input()->when(FL_WHEN_CHANGED);
	page->input->input()->callback([](Fl_Widget*, void* p) { static_cast<EntityBrowser*>(p)->UpdatePageId(); }, this);
	page->input->resizable(nullptr);

	// previous page button decrements the current page id
	page->prev_button = ut::MakeUnique<Button>(page->input->x() - input_height,
	                                           page->input->y(),
	                                           input_height,
	                                           input_height);
	page->prev_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateArrow(input_height, input_height,
	                                                                  icon_color,
	                                                                  icon_margin, true, false)));
	page->prev_button->SetCallback([&]() { page->page_id.Store(page->page_id.Read() - 1); ImmediateUpdate(); });

	// first page button sets the current page id to 1
	page->first_button = ut::MakeUnique<Button>(page->prev_button->x() - input_height,
	                                            page->input->y(),
	                                            input_height,
	                                            input_height);
	page->first_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateArrow(input_height, input_height,
	                                                                   icon_color,
	                                                                   icon_margin, true, true)));
	page->first_button->SetCallback([&]() { page->page_id.Store(1); ImmediateUpdate(); });

	// next page button increments the current page id
	page->next_button = ut::MakeUnique<Button>(page->input->x() + page->input->w(),
	                                           page->input->y(),
	                                           input_height,
	                                           input_height);
	page->next_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateArrow(input_height, input_height,
	                                                                  icon_color,
	                                                                  icon_margin, false, false)));
	page->next_button->SetCallback([&]() { page->page_id.Store(page->page_id.Read() + 1); ImmediateUpdate(); });

	// last page button sets the current page id to the value equal to the page count
	page->last_button = ut::MakeUnique<Button>(page->next_button->x() + page->next_button->w(),
	                                           page->input->y(),
	                                           input_height,
	                                           input_height);
	page->last_button->SetIcon(ut::MakeShared<Icon>(Icon::CreateArrow(input_height, input_height,
	                                                                  icon_color,
	                                                                  icon_margin, false, true)));
	page->last_button->SetCallback([&]() { page->page_id.Store(page->page_count.Read()); ImmediateUpdate(); });

	// this description box displays the number of pages
	page->page_count_box = ut::MakeUnique<Fl_Box>(page->x(),
	                                              page->y(),
	                                              page->first_button->x() - page->x(),
	                                              PageControls::skHeight);
	page->page_count_box->box(FL_NO_BOX);
	page->page_count_box->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

	// this description box displays the number of entities
	page->entity_count_box = ut::MakeUnique<Fl_Box>(page->last_button->x() + page->last_button->w(),
	                                                page->y(),
	                                                page->x() + page->w() - page->last_button->x() - page->last_button->w(),
	                                                PageControls::skHeight);
	page->page_count_box->box(FL_NO_BOX);
	page->entity_count_box->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

	// finish group
	page->end();
}

// Creates an array of EntityView::Proxy from the provided entity map that
// will be used to update UI component views on the next UI tick.
//    @param access - reference to the object providing access to the
//                    desired components.
void EntityBrowser::MapEntitiesToUiProxies(ComponentAccessGroup& group_access)
{
	ut::ScopeLock lock(mutex);

	// clear current views array
	pending_views.Reset();

	// filter entities
	ComponentAccess access = group_access.GetAccess(0);
	FilterEntities(access);
	const size_t filtered_entity_count = filter_cache.Count();

	// initialize page data
	const PageView page_view = PreparePage();

	// allocate new entity views
	pending_views.Resize(page_view.entity_count);

	// serialize entities into the appropriate ui views
	for (size_t i = 0; i < page_view.entity_count; i++)
	{
		const Entity::Id entity_id = filter_cache[page_view.first_filtered_id + i];
		ut::Optional<const Entity&> entity = access.FindEntity(entity_id);
		UT_ASSERT(entity.HasValue());

		EntityView::Proxy& view_proxy = pending_views[i];
		view_proxy.id = entity_id;

		const size_t component_count = entity->CountComponents();
		for (size_t component_index = 0; component_index < component_count; component_index++)
		{
			const ut::DynamicType::Handle& component_type = entity->GetComponentByIndex(component_index);
			ut::Optional<ComponentMap&> component_map = access.GetMap(component_type);
			UT_ASSERT(component_map.HasValue());

			ut::Optional<Component&> component = component_map->Find(entity_id);
			UT_ASSERT(component.HasValue());

			ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(component.Get(),
			                                                          component->Identify().GetName(),
			                                                          ut::meta::Info::CreateComplete());
			view_proxy.components.Add(ComponentView::Proxy{ ut::Move(snapshot), component_type, entity_id });
		}
	}
}

// Updates UI representation of all entities.
void EntityBrowser::UpdateUi()
{
	// UI proxies must be updated in asynchronous UpdateUi() call
	// only once after every UpdateEntities call. But Fl::awake() can produce
	// multiple calls so they are guarded with @requires_ui_update lock.
	bool& needs_update = requires_ui_update.Lock();
	const bool skip = !needs_update || !this->visible();
	needs_update = false;
	requires_ui_update.Unlock();

	if (skip)
	{
		return;
	}

	ut::ScopeLock lock(mutex);

	FixScrollPosition();
	InvalidateAllViews();
	UpdateViews();
	RemoveInvalidViews();
	RepositionViews();
	UpdateComponentFilterUi();
	UpdatePageControls();
	view_area->redraw();
	pending_views.Reset();
}

// Searches for the entity view by id.
//    @param entity_id - identifier of the desired entity.
//    @return - optional reference to the entity view, or nothing
//              if not found.
ut::Optional<EntityView&> EntityBrowser::FindView(Entity::Id entity_id)
{
	const size_t view_count = entity_views.Count();
	for (size_t i = 0; i < view_count; i++)
	{
		ut::UniquePtr<EntityView>& view = entity_views[i];
		if (entity_id == view->GetEntityId())
		{
			return view.GetRef();
		}
	}

	return ut::Optional<EntityView&>();
}

// Creates a new entity view from the provided proxy.
//    @return - a reference to the new view.
EntityView& EntityBrowser::AddEntityView(EntityView::Proxy& proxy)
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

	// insert the view using the 'bubble' principle to retain sorting
	for (size_t i = 0; i < entity_views.Count(); i++)
	{
		if (new_view->GetEntityId() < entity_views[i]->GetEntityId())
		{
			entity_views.Insert(i, ut::Move(new_view));
			return entity_views[i].GetRef();
		}
	}

	// push back if no holes were find in the list of entities
	entity_views.Add(ut::Move(new_view));
	return entity_views.GetLast().GetRef();
}

// Marks all enitity views as 'invalid'.
void EntityBrowser::InvalidateAllViews()
{
	const size_t prev_view_count = entity_views.Count();
	for (size_t i = 0; i < prev_view_count; i++)
	{
		entity_views[i]->is_valid = false;
	}
}

// Updates entity views with @pending_views proxy array.
void EntityBrowser::UpdateViews()
{
	ut::Optional<Entity::Id> entity_to_highlight = new_entity_id.Get();

	const ut::uint32 pending_entity_count = static_cast<ut::uint32>(pending_views.Count());
	for (ut::uint32 i = 0; i < pending_entity_count; i++)
	{
		EntityView::Proxy& pending_view = pending_views[i];
		const bool highlight = entity_to_highlight && entity_to_highlight.Get() == pending_view.id;

		ut::Optional<EntityView&> existing_view = FindView(pending_view.id);
		if (existing_view)
		{
			existing_view->Update(pending_view);
			existing_view->is_valid = true;
		}
		else
		{
			EntityView& new_view = AddEntityView(pending_view);
			new_view.MarkNew(highlight);
		}
	}

	// reset new entity id
	new_entity_id.Set(ut::Optional<Entity::Id>());
}

// Removes all entity views having 'invalid' flag set.
void EntityBrowser::RemoveInvalidViews()
{
	const size_t view_count = entity_views.Count();
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

		ImmediateUpdate();
	}
}

// Updates y-position of all entity view.
void EntityBrowser::RepositionViews()
{
	int y_position = 0;
	const size_t view_count = entity_views.Count();
	for (size_t i = 0; i < view_count; i++)
	{
		EntityView& view = entity_views[i].GetRef();
		view.position(0, y_position);

		y_position += view.h();
	}

	// fix scroll position if it exeeds the bounds of the view area
	const int scroll_val = static_cast<int>(view_area->scrollbar.value() + ut::Precision<double>::epsilon);
	if (scroll_val != 0 && y_position < scroll_val)
	{
		const int new_scroll_position = ut::Max<int>(0, y_position - view_area->h());
		view_area->scroll_position = ut::Vector<2, int>(0, new_scroll_position);
	}
}

// Updates size/position of all internal widgets. This function is supposed
// to be called when one internal widget changes its size and all other widgets
// must be shifted up or down.
void EntityBrowser::UpdateSize()
{
	UpdateFilterSize();

	view_area->resize(0, entity_controls.group->h(), this->w(),
	                  this->h() - entity_controls.group->h() - page->h());

	FixScrollPosition();

	const int view_width = CalculateEntityViewWidth();
	const size_t view_count = entity_views.Count();
	for (size_t i = 0; i < view_count; i++)
	{
		EntityView& view = entity_views[i].GetRef();
		view.size(view_width, view.h());
		view.UpdateSize();
	}
	
	RepositionViews();

	redraw();
}

// Updates all filter widgets with correct position and size.
void EntityBrowser::UpdateFilterSize()
{
	const bool component_filter_expanded = entity_controls.components.expand_button->IsOn();
	const int component_filter_height = component_filter_expanded ?
	                                    entity_controls.components.group->h() : 0;
	const int filter_group_height = EntityControls::skFilterInputHeight +
	                                EntityControls::ComponentFilter::skCollapseButtonHeight +
	                                component_filter_height +
	                                EntityControls::skDelimiterHeight * 2 +
	                                EntityControls::skMargin * 5;

	entity_controls.group->resize(EntityBrowser::skOffset,
	                              0,
	                              w() - EntityBrowser::skOffset,
	                              filter_group_height);

	// delimiter top box
	entity_controls.delimiter_top_box->resize(0,
	                                          entity_controls.group->y() + EntityControls::skMargin,
	                                          w(),
	                                          EntityControls::skDelimiterHeight);

	// filter input
	entity_controls.filter_input->resize(entity_controls.group->x(),
	                                     entity_controls.delimiter_top_box->y() +
	                                     entity_controls.delimiter_top_box->h() +
	                                     EntityControls::skMargin,
	                                     entity_controls.group->w() - EntityControls::skFilterInputHeight * 2,
	                                     EntityControls::skFilterInputHeight);

	// add entity button
	entity_controls.add_entity_button->resize(entity_controls.group->x() + entity_controls.filter_input->w(),
	                                          entity_controls.filter_input->y(),
	                                          EntityControls::skFilterInputHeight,
	                                          EntityControls::skFilterInputHeight);

	// clear filter button
	entity_controls.clear_filter_button->resize(entity_controls.group->x() +
	                                            entity_controls.filter_input->w() +
	                                            EntityControls::skFilterInputHeight,
	                                            entity_controls.filter_input->y(),
	                                            EntityControls::skFilterInputHeight,
	                                            EntityControls::skFilterInputHeight);

	// filter expand
	entity_controls.components.expand_button->resize(entity_controls.group->x(),
	                                                 entity_controls.filter_input->y() +
	                                                 entity_controls.filter_input->h() +
	                                                 EntityControls::skMargin,
	                                                 w() - EntityBrowser::skOffset * 2,
	                                                 EntityControls::ComponentFilter::skCollapseButtonHeight);

	// components
	entity_controls.components.group->resize(entity_controls.group->x(),
	                                         entity_controls.components.group->y(),
	                                         entity_controls.group->w(),
	                                         entity_controls.components.group->h());

	// delimiter bottom box
	entity_controls.delimiter_bottom_box->resize(0,
	                                             entity_controls.group->y() +
	                                             entity_controls.group->h() -
	                                             EntityControls::skDelimiterHeight -
	                                             EntityControls::skMargin,
	                                             w(),
	                                             EntityControls::skDelimiterHeight);

	if (component_filter_expanded)
	{
		entity_controls.components.group->show();
	}
	else
	{
		entity_controls.components.group->hide();
	}
}

// Returns an array of accumulated commands pending to be processed.
CmdArray EntityBrowser::FlushCommands()
{
	ut::ScopeLock lock(mutex);

	CmdArray cmd = ut::Move(pending_commands.Lock());
	pending_commands.Unlock();

	const size_t view_count = entity_views.Count();
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

// Adds a new entity on the next UpdateEntities() call.
void EntityBrowser::AddEntity()
{
	ut::UniquePtr<CmdAddEntity> cmd = ut::MakeUnique<CmdAddEntity>();

	typedef void AddEntityCb(const CmdAddEntity::AddResult&);
	cmd->Connect(ut::MemberFunction<EntityBrowser, AddEntityCb>(this, &EntityBrowser::AddEntityCallback));

	ut::ScopeSyncLock<CmdArray> cmd_lock(pending_commands);
	cmd_lock.Get().Add(ut::Move(cmd));

	ImmediateUpdate();
}

// Updates entity browser content after an entity was added.
void EntityBrowser::AddEntityCallback(const CmdAddEntity::AddResult& add_result)
{
	ImmediateUpdate();

	if (add_result)
	{
		const Entity::Id id = add_result.Get();
		entity_id_filter.Set(ut::String("#") + ut::Print(id));
		new_entity_id.Set(id);
		ResetComponentFilter();
	}
}

// Clears entity filter input field.
void EntityBrowser::ClearFilter()
{
	entity_id_filter.Set(ut::String());
	ResetComponentFilter();
}

// Updates @entity_controls.filter string with
// the value of the input widget.
void EntityBrowser::UpdateFilterInput()
{
	ut::String filter_text(entity_controls.filter_input->value());
	entity_id_filter.Set(ut::Move(filter_text));
	page->page_id.Store(1);
	ImmediateUpdate();
}

// Updates all UI checkboxes according to the @entity_controls.components value,
// also updates component filter expand button description and color.
void EntityBrowser::UpdateComponentFilterUi()
{
	const ut::Array<ut::DynamicType::Handle> component_filter = component_type_filter.Get();

	// update all checkboxes
	for (ut::UniquePtr<Fl_Check_Button>& checkbox : entity_controls.components.checkboxes)
	{
		ut::Result<const ut::DynamicType&, ut::Error> component_type_result =
			ut::Factory<Component>::GetType(ut::String(checkbox->label()));
		if (!component_type_result)
		{
			continue;
		}

		const ut::DynamicType& component_type = component_type_result.Get();

		typedef ut::Array<ut::DynamicType::Handle>::ConstIterator ComponentFilterIterator;
		const ut::Optional<ComponentFilterIterator> find_result = ut::Find(component_filter.Begin(),
		                                                                   component_filter.End(),
		                                                                   component_type.GetHandle());
		if (find_result)
		{
			checkbox->value(1);
		}
		else
		{
			checkbox->value(0);
		}
	}

	// update expand button description and color
	const ut::Color<3, ut::byte> expand_button_color = component_filter.IsEmpty() ?
	                                                   entity_controls.components.expand_empty_bkg_color :
	                                                   entity_controls.components.expand_bkg_color;
	entity_controls.components.counter_text = component_filter.IsEmpty() ? ut::String("") :
	                                          (ut::String("Filtered components: ") + ut::Print(component_filter.Count()));
	entity_controls.components.counter_box->label(entity_controls.components.counter_text.GetAddress());
	entity_controls.components.expand_button->SetBackgroundColor(Button::state_release,
	                                                             ConvertToFlColor(expand_button_color));
	entity_controls.components.expand_button->SetBackgroundColor(Button::state_hover,
	                                                             ConvertToFlColor(expand_button_color));
}

// Updates page id from the input field.
void EntityBrowser::UpdatePageId()
{
	const ut::String page_str(page->input->value());

	if (page_str.Length() == 0)
	{
		page->page_id.Store(1);
	}
	else
	{
		page->page_id.Store(ut::Scan<ut::uint32>(page->input->value()));
	}

	ImmediateUpdate();
}

// Updates all page controls.
void EntityBrowser::UpdatePageControls()
{
	if (Fl::focus() != page->input->input() ||
	    ut::StrLen<char>(page->input->value()) != 0)
	{
		const ut::String page_id_str = ut::Print(page->page_id.Read());
		page->input->value(page_id_str.GetAddress());
	}

	const ut::String filter = entity_id_filter.Get();
	if (filter != entity_controls.filter_input->value())
	{
		entity_controls.filter_input->value(filter.GetAddress());
	}

	ut::String page_count_str = ut::String("Pages: ") + ut::Print(page->page_count.Read());
	ut::String entity_count_str = ut::String("Entities: ") + ut::Print(page->entity_count.Read());
	page->page_count_buffer = ut::MakeUnique<ut::String>(ut::Move(page_count_str));
	page->entity_count_buffer = ut::MakeUnique<ut::String>(ut::Move(entity_count_str));
	page->page_count_box->label(page->page_count_buffer->GetAddress());
	page->entity_count_box->label(page->entity_count_buffer->GetAddress());
}

// Filters provided entities and stores filtered
// indices to the @filter_cache.
//    @param access - reference to the object providing access to the
//                    desired components.
void EntityBrowser::FilterEntities(ComponentAccess& access)
{
	const ut::String filter = entity_id_filter.Get();
	const bool filter_is_not_empty = filter.Length() != 0;
	const bool exact_id_request = filter_is_not_empty && filter[0] == '#';

	ut::Optional<size_t> filtered_scroll_index;
	IterativeComponentSet::Iterator entity_it;
	
	// get component type filter
	const ut::Array<ut::DynamicType::Handle> component_filter = component_type_filter.Get();

	// iterate all entities
	filter_cache.Reset();
	for (entity_it = access.BeginEntities(); entity_it != access.EndEntities(); ++entity_it)
	{
		const Entity::Id entity_id = entity_it->GetFirst();
		const Entity& entity = entity_it->GetSecond();

		// filter by component type
		bool has_absent_components = false;
		for (ut::DynamicType::Handle component_type : component_filter)
		{
			if (!entity.HasComponent(component_type))
			{
				has_absent_components = true;
				break;
			}
		}

		if (!component_filter.IsEmpty() && has_absent_components)
		{
			continue;
		}

		// find the name component
		ut::Optional<const NameComponent&> name_component;
		if (entity.HasComponent(ut::GetPolymorphicHandle<NameComponent>()))
		{
			ut::Optional<ComponentMap&> name_map = access.GetMap<NameComponent>();
			if (name_map)
			{
				ut::Optional<Component&> name_find_result = name_map->Find(entity_id);
				if (name_find_result)
				{
					name_component = static_cast<const NameComponent&>(name_find_result.Get());
				}
			}
		}

		// check if the name matches the filter query
		const bool name_match = name_component && ut::StrStr(name_component->name.GetAddress(), filter.GetAddress()) != nullptr;

		// check if the id of the entity fully matches the filter query
		const bool id_match = ut::Print(entity_id) == filter;
		const bool exact_id_match = (ut::String("#") + ut::Print(entity_id)) == filter;

		// weed out filtered values
		if (exact_id_request && !exact_id_match)
		{
			continue;
		}
		else if (!exact_id_request && filter_is_not_empty && !id_match && !name_match)
		{
			continue;
		}

		// find the position to insert a new value
		size_t insert_position = 0;
		for (size_t i = filter_cache.Count(); i-- > 0;)
		{
			if (entity_id > filter_cache[i])
			{
				insert_position = i + 1;
				break;
			}
		}

		// add filtered value index to the cache
		if (!filter_cache.Insert(insert_position, entity_id))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
}

// Calculates the first index of the filtered entities and the number of
// entities to be shown on the current page.
//    @return - the EntityBrowser::PageView structure value.
EntityBrowser::PageView EntityBrowser::PreparePage()
{
	const size_t filtered_entity_count = filter_cache.Count();
	const ut::uint32 page_count = static_cast<ut::uint32>(filtered_entity_count / page->capacity + 1);

	// find the current page index
	ut::uint32 current_page_id = page->page_id.Read();

	if (current_page_id == 0)
	{
		current_page_id = 1;
	}
	else if (current_page_id * page->capacity > filtered_entity_count)
	{
		current_page_id = page_count;
	}

	// update @page controls
	page->page_id.Store(current_page_id);
	page->page_count.Store(page_count);
	page->entity_count.Store(static_cast<ut::uint32>(filtered_entity_count));

	// initialize and return a page view
	PageView view;
	view.first_filtered_id = --current_page_id * page->capacity;
	view.entity_count = ut::Min<size_t>(filtered_entity_count - view.first_filtered_id, page->capacity);
	return view;
}

//----------------------------------------------------------------------------//
// Centers the page index input widget.
void EntityBrowser::PageControls::resize(int px, int py, int mw, int mh)
{
	const int widget_size = input->h();
	const int input_width = input->w();
	const int ppx = px + mw / 2 - input_width / 2;
	const int ppy = py + mh / 2 - widget_size / 2;

	Fl_Group::resize(px, py, mw, mh);

	input->resize(ppx,
	              ppy,
	              input_width,
	              widget_size);

	prev_button->resize(ppx - widget_size,
	                    ppy,
	                    widget_size,
	                    widget_size);
	first_button->resize(ppx - widget_size * 2,
	                     ppy,
	                     widget_size,
	                     widget_size);
	next_button->resize(ppx + input_width,
	                    ppy,
	                    widget_size,
	                    widget_size);
	last_button->resize(ppx + input_width + widget_size,
	                    ppy,
	                    widget_size,
	                    widget_size);

	page_count_box->resize(px, ppy,
	                       ppx - widget_size * 2 - px,
	                       widget_size);
	entity_count_box->resize(ppx + input_width + widget_size * 2,
	                         ppy,
	                         px + mw - ppx - input_width - widget_size * 2,
	                         widget_size);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
