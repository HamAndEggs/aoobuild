# About
This project is an alternative (not a replacement) to makefiles for when compling c/c++ applications. 

So I have taken a leaf out of the Microsoft Visual Studio book and create a stand alone app that uses a single file to represent project consisting of one or more build targets.

The aim is to have a simple file to represent a c/c++ project that is easy to maintain and has all the features that I have wanted over the years. 

## Current features
* Targets applications and static libraries.
* Full dependency checking.
* Project references, if an app refers to a library and the source of that library has changed it will build that library before continuing with building the application, just as you would expect. 
* Multithread compiling.
* Builtin build environment defines to help with build time and version generation.
* Single process used during entire build process that allows for increased speed of dependency checking between source files.
* Does not create extra files to manage the project and so will not clutter up your repository. Just uses the **one** project file for each project.
* Resource file support with file API and compression, think something like res folder for Android apps.
* Json file format allowing intergration with external editors where the editor can add it’s own values to the file and the tool will ignore them.
* Can be used as a shebang in a c/c++ file so that this source file is now execuatable. As if it was a shell script.

## Motivation
I have always found it frustrating to have to create makefiles. Mostly because the syntax means that when you go back to a makefile after many months or even years you have to learn the syntax all over again. I find their maintability hard. Also I have never got the auto dependency checking to work.

I also want to add features that are only applicable when creating applications such as compiling resource files, automatic date stamping and version marking.

# Installation
Just run the build_and_install.sh and it will compile and copy it to /usr/share/bin and you’re ready to rock and roll.

# Usage
There are some example projects included so you can see how it can be used as well as a way to validate any new build as I work on the project. The project file can be very minimal listing just the files to complile. The other values required will have defaults that will be used. The application has an option, decribed below, that can be used to populate the missing project file options with their defaults.

## Source files

```json
{
	"source_files":
	[
		"./main.cpp"		
	]
}
```

The "source_files" member can either be an array, as above or an object with many children objects. The former is intended for use in an IDE where source files can be grouped with filers or virtual folders. Below is an example.

```json
{
	"source_files":
	{
		"./main.cpp":"main",
		"./gfx/particles.cpp":"fx",
		"./sprites/ship.cpp":"fx",
		"./mp3.cpp":"music",
	}
}
```

## Configurations
The 'dependency' example project shows a use of the "configuration" object in the project Json file. The configuration object can have any number of children objects defining the different configurations that can be build, the most common would be release and debug. There is no reserved configuration names and you're free to name them what you like although I expect most will use 'release' and 'debug'. Below is an example...

```json
{
	"configurations":
	{
		"release":
		{
			"target":"executable",
			"output_name":"dependency",
			"optimisation":2,
			"standard":"c++14"
		},
		"debug":
		{
			"target":"executable",
			"output_name":"dependency_dbg",
			"optimisation":0,
			"standard":"c++14"
		}
	},
    "source_files":
	[
		"./main.cpp"
		
	]
}
```

## shebang (executable source files)
All you need to do is add #!/usr/bin/appbuild -# to a source file and you're good to go.
You can pass params to your executable source file and capture shell input as well as output to the shell. It runs as if it had been compiled. This is because, under the covers, it has.
Currently the code only links against stdc++ and pthread. I later plan to allow customisation of this.
There is an example in ./examples/shebang/ with a cpp file and a python3 file that do the same. Run these and see the difference in preformace!

To add aditional libraries to the linker stage use the #APPBUILD_LIBS pragma. An example is in ./appbuild/examples/shebang/curses-hello-world.c

## Work left to do
* Implement a resource file system.
* Add missing compiler options as and when I discover them or I am informed of them.

## Contributors
Richard e Collins

## License
Copyright (C) 2017, Richard e Collins.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
