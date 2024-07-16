target_sources(
    glare_app # glare_core
    
    PRIVATE

    "input_handler.cpp"
    "input_profile_impl.cpp"
    "virtual_button.cpp"
    "events.cpp"

    "gamepad.cpp"
    "gamepad_manager.cpp"
    "gamepad_profile.cpp"
    "gamepad_state.cpp"

    "keyboard.cpp"
    "keyboard_events.cpp"
    "keyboard_profile.cpp"
    "keyboard_state.cpp"

    "mouse.cpp"
    "mouse_profile.cpp"
    "mouse_state.cpp"
)