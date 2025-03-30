# Rippleator VST Plugin

A physical modeling audio effect plugin that simulates sound propagation through a 2D chamber filled with fluid or gas.

## Overview

Rippleator models a rectangular 2D chamber with customizable medium properties. The plugin simulates:
- Sound input through a speaker placed on the chamber wall
- Sound propagation through the medium with configurable density
- Sound capture via up to 3 microphones placed on chamber walls

## Features

- Physical modeling of sound wave propagation in 2D space
- Adjustable medium density
- Configurable speaker and microphone positions
- Real-time visualization of wave propagation
- Up to 3 microphone outputs for stereo/multi-channel effects

## Development

This project is built using the JUCE framework for VST plugin development.

### Project Structure

- `Source/` - Main source code
  - `DSP/` - Digital signal processing algorithms
  - `GUI/` - User interface components
  - `Models/` - Physical modeling implementation
  - `Utils/` - Utility functions and helpers
- `JuceLibraryCode/` - JUCE library integration

## Building

*Instructions for building the plugin will be added once the project is set up with JUCE.*

## License

*License information to be determined*
