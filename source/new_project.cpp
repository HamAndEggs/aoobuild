#include <sys/stat.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "json.h"

#include "new_project.h"
#include "misc.h"
#include "dependencies.h"
#include "source_files.h"
#include "configuration.h"
#include "project.h"
#include "logging.h"

typedef struct stat FileStats;

namespace appbuild{
//////////////////////////////////////////////////////////////////////////
StringVec FindSourceFiles(const std::string& pPath,int pLoggingMode)
{
    StringVec FoundFiles;

    DIR *dir = opendir(pPath.c_str());
    if(dir)
    {
        struct dirent *ent;
        while((ent = readdir(dir)) != nullptr)
        {
            const std::string fname = ent->d_name;
            if( fname != "." && fname != ".." )
            {
                if( pLoggingMode == LOG_VERBOSE )
                {
                    std::cout << "FindFiles \'" << fname << "\' found" << std::endl;
                }
                if( ent->d_type == DT_DIR )
                {
                    StringVec subFolderFiles = FindSourceFiles(pPath + fname + "/",pLoggingMode);
                    for( auto f : subFolderFiles )
                    {
                        FoundFiles.push_back(f);
                    }
                }
                else if( ent->d_type == DT_REG )
                {
                    if( GetExtension(fname) == "cpp" || GetExtension(fname) == "c" )
                    {
                        const std::string pathedFile = pPath + fname;
                        FoundFiles.push_back(pathedFile);
                        if( pLoggingMode == LOG_VERBOSE )
                        {
                            std::cout << pathedFile << std::endl;
                        }
                    }

                }
            }
        }
        closedir(dir);
    }
    else if( pLoggingMode == LOG_VERBOSE )
    {
        std::cout << "FindFiles \'" << pPath << "\' was not found or is not a path." << std::endl;
    }

    return FoundFiles;
}

int CreateNewProject(const std::string& pNewProjectName,int pLoggingMode)
{
    // Check name does not use any pathing characters.
    if( pNewProjectName.find('/') != std::string::npos )
    {
        std::cout << "--new-project arg contained the disallowed character '/'" << std::endl;
        return EXIT_FAILURE;
    }

    // Name good, now make sure folder does not already exists.
    const std::string projectPath = "./" + pNewProjectName + "/";
    const std::string pathedProjectFilename = projectPath + pNewProjectName + ".proj";

    // Make sure we're not going to overwrite something that is already there.
    if( FileExists(pathedProjectFilename) )
    {
        std::cout << "Requested project already exists, can not make new project file " << pathedProjectFilename << std::endl;
        return EXIT_FAILURE;
    }

    SourceFiles SourceFiles(GetPath(pathedProjectFilename),pLoggingMode);

    if( DirectoryExists(projectPath) )
    {
        // Folder already here, so look for source files to add.
        if( pLoggingMode == LOG_VERBOSE )
        {
            std::cout << "Searching for c/c++ files in " << projectPath << std::endl;
        }

        const StringVec foundFiles = FindSourceFiles(projectPath,pLoggingMode);
        // We need to remove the path of the project from the string before adding to the source files.
        for( auto f : foundFiles )
        {
            const std::string sourceFile = ReplaceString(f,projectPath,"");
            SourceFiles.AddFile(sourceFile);
        }

    }
    else
    {
        if( MakeDir(projectPath) == false )
        {
            std::cout << "Failed to create path for the new project to reside in, [" << projectPath << "]" << std::endl;
            return EXIT_FAILURE;
        }

        // Going to be making a new folder, so add the folder and a new empty source file.
        const std::string defaultMainName = pNewProjectName + ".cpp";
        const std::string pathedSourceFilename = projectPath + defaultMainName;

        std::ofstream sourceFile(pathedSourceFilename);
        if( sourceFile.is_open() )
        {
            // Now write a hello world....

            sourceFile << "#include <iostream>" << std::endl;
            sourceFile << std::endl;
            sourceFile << "int main(int argc, char *argv[])" << std::endl;
            sourceFile << "{" << std::endl;
            sourceFile << "// Say hello to the world!" << std::endl;
            sourceFile << "    std::cout << \"Hello world, a skeleton app generated by appbuild.\" << std::endl;" << std::endl;
            sourceFile << std::endl;
            sourceFile << "// Display the constants defined by app build. " << std::endl;
            sourceFile << "    std::cout << \"Build date and time \" << APP_BUILD_DATE_TIME << std::endl;" << std::endl;
            sourceFile << "    std::cout << \"Build date \" << APP_BUILD_DATE << std::endl;" << std::endl;
            sourceFile << "    std::cout << \"Build time \" << APP_BUILD_TIME << std::endl;" << std::endl;
            sourceFile << std::endl;
            sourceFile << "// And quit" << std::endl;
            sourceFile << "    return 0;" << std::endl;
            sourceFile << "}" << std::endl;
            sourceFile << std::endl;

            sourceFile.close();

            // Now the file has been written we can add it. Would fail if done before.
            if( SourceFiles.AddFile(defaultMainName) == false )
            {
                std::cout << "Failed to add default cpp file to the project, [" << defaultMainName << "]" << std::endl;
                return EXIT_FAILURE;
            }

        }
        else
        {
            std::cout << "Failed to create default source file for new project, [" << pathedSourceFilename << "]" << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "SourceFiles.size() == " << SourceFiles.size() << std::endl;

    ConfigurationsVec configs;
	std::shared_ptr<Configuration> newConfig = nullptr;
	// Add a release and debug configuration.
	
	newConfig = std::make_shared<Configuration>("release",pNewProjectName,projectPath,pLoggingMode,true,"2","0");
	newConfig->AddDefine("NDEBUG");
	newConfig->AddDefine("RELEASE_BUILD");
    configs.push_back(newConfig);

	newConfig = std::make_shared<Configuration>("debug",pNewProjectName,projectPath,pLoggingMode,false,"0","2");
	newConfig->AddDefine("DEBUG_BUILD");
    configs.push_back(newConfig);

    // Now build the project object.
    Project newProject(pathedProjectFilename,SourceFiles,configs,pLoggingMode);
    if( newProject == false )
    {
        std::cout << "Failed to create default project file, [" << pathedProjectFilename << "]" << std::endl;        
        return EXIT_FAILURE;
    }

    rapidjson::Document jsonOutput;
    jsonOutput.SetObject(); // Root object is an object not an array.
    newProject.Write(jsonOutput);
    if( appbuild::SaveJson(pathedProjectFilename,jsonOutput) )
    {
        std::cout << "New project file saved" << std::endl;
    }
    else
    {
        std::cout << "Failed to write to the destination file, [" << pathedProjectFilename << "]" << std::endl;
    }

    return EXIT_SUCCESS;    
}

//////////////////////////////////////////////////////////////////////////
};//namespace appbuild
