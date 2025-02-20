#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include "tools.h"
#include "functions.c"

#ifdef _WIN32
# define mkdir(X, Y) mkdir(X)
#endif


enum TpathIndexes
{
    folderPath,
    projectPath,
    filePath,
    filename,
    layerPath,
    bpmPath,
    iniPath,
    evdPath,
    ppdPath,
    scdPath,
    csinputPath,
    divascriptPath,
    targetDivascriptPath,
    targetCsinputPath,
    targetBpmPath,
    targetLayerPath,
    scriptFolderPath,
    layerFolderPath,
    soundFolderPath,
    ressourceFolderPath,
    soundsetPath,
    targetSoundsetPath,
    targetScdPath,
    soundPath,
    targetSoundPath,
    ppdprojPath,
    outputFolderPath,
    difficultyChar,
    TPATH_LENGTH,
};

typedef char* Tpath[TPATH_LENGTH];

/*
Fills path strings by extracting the current folder location
*/
int initPath(int argc, char** argv, int choice, int difficulty, Tpath path, char** error)
{
    //add path
    if (argc > 0)
    {
        path[folderPath] = extractFolderPath(argv[0]);


        if (argc > 1)
        {
            switch(difficulty)
            {
                case 1:
                    path[difficultyChar] = strdup("Easy");
                    break;
                case 2:
                    path[difficultyChar] = strdup("Normal");
                    break;
                case 3:
                    path[difficultyChar] = strdup("Hard");
                    break;
                case 4:
                    path[difficultyChar] = strdup("Extreme");
                    break;
                case 5:
                    path[difficultyChar] = strdup("Base");
                    break;
            }
            
            char *outputFolder;
            switch(choice)
            {
                case 1:
                    outputFolder = "outputProjects";
                    break;
                case 2:
                    outputFolder = "outputLayers";
                    break;
                case 3:
                    outputFolder = "outputScores";
                    break;
            }
            asprintf(&path[outputFolderPath], "%s%s", path[folderPath], outputFolder);

            path[filePath] = strdup(argv[1]);

            #ifdef _WIN32
            if (containsUnsupportedChar(path[filePath]))
            {
                *error = "DSC file path contains unsupported characters (most likely japanese text).\nPlease make sure file and sub folders are not in japanese\n";
                return 1;
            }
            if (containsUnsupportedChar(path[folderPath]))
            {
                *error = "DSCtoPPD.exe path contains unsupported characters (most likely japanese text)\nPlease make sure sub folders are not in japanese\n";
                return 1;
            }
            #endif

            path[filename] = extractFileName(path[filePath], 0);

            asprintf(&path[projectPath],            "%s/%s",                    path[outputFolderPath], path[filename]);
            asprintf(&path[layerPath],              "%s/%s.ppd",                path[projectPath], path[filename]);
            asprintf(&path[ppdprojPath],            "%s.ppdproj",               path[projectPath]);
            asprintf(&path[iniPath],                "%s/data.ini",              path[projectPath]);
            asprintf(&path[evdPath],                "%s/evd.txt",               path[projectPath]);
            asprintf(&path[ppdPath],                "%s/%s.ppd",                path[projectPath], path[difficultyChar]);
            asprintf(&path[targetScdPath],          "%s/%s.scd",                path[projectPath], path[difficultyChar]);
            asprintf(&path[scdPath],                "%sData/scd.scd",           path[folderPath]);
    
            asprintf(&path[soundPath],              "%sData/Sound.wav",         path[folderPath]);
            asprintf(&path[soundsetPath],           "%sData/soundset.txt",      path[folderPath]);
            asprintf(&path[soundFolderPath],        "%s/Sound",                 path[projectPath]);
            asprintf(&path[targetSoundPath],        "%s/Sound/sound.wav",       path[projectPath]);
            asprintf(&path[targetSoundsetPath],     "%s/soundset.txt",          path[projectPath]);

            asprintf(&path[csinputPath],            "%sData/CSInput.fsml",      path[folderPath]);
            asprintf(&path[divascriptPath],         "%sData/DivaScript.fsml",   path[folderPath]);
            asprintf(&path[bpmPath],                "%s/BPM.fsml",              path[projectPath]);
            asprintf(&path[scriptFolderPath],       "%s/%s_Scripts",            path[projectPath],  path[difficultyChar]);
            asprintf(&path[targetCsinputPath],      "%s/CSInput.fsml",          path[scriptFolderPath]);
            asprintf(&path[targetDivascriptPath],   "%s/DivaScript.fsml",       path[scriptFolderPath]);
            asprintf(&path[targetBpmPath],          "%s/BPM.fsml",              path[scriptFolderPath]);

            asprintf(&path[layerFolderPath],        "%s/%s_Layers",             path[projectPath],  path[difficultyChar]);
            asprintf(&path[targetLayerPath],        "%s/layer0.ppd",            path[layerFolderPath]);
        }
        else
        {
            *error = "Missing file path\n";

            return 1;
        }
    }
    else
    {
        *error = "Missing executable path, can't create folders\n";

        return 1;
    }

    return 0;
}

/*
Free all strings that had memory allocations in Tpath structure
*/
int freePath(Tpath path)
{
    for (int i=0; i<TPATH_LENGTH; i++)
    {
        if (path[i] != NULL)
        {
            free(path[i]);
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    char *errmsg;
    int operation = 0, difficulty = 0;
    Tpath path = {0};
    Tchart chart = {0};

    //Ask for layer, project or score
    while ((operation <= 0) || (operation >= 4))
    {
        printf("Input number;\n1 : PPD project\n2 : PPD layer file (no BPM changes will be ported)\n3 : PPD score (playable chart)\n");
        if (scanf("%d",&operation) != 1) {
            errmsg = "Invalid input. Aborting...\n";
            goto error;
        }
        fflush(stdin);
    }

    //Ask for difficulty
    while ((difficulty <= 0) || (difficulty >= 6))
    {
        printf("Input number;\n1 : Easy\n2 : Normal\n3 : Hard\n4 : Extreme\n5 : Base\n");
        if (scanf("%d",&difficulty) != 1) {
            errmsg = "Invalid input. Aborting...\n";
            goto error;
        }
        fflush(stdin);
    }

    if (initPath(argc, argv, operation, difficulty, &path, &errmsg))
    {
        freePath(&path);
        goto error;
    }

    initChart(&chart);

    mkdir(path[outputFolderPath], 0777);
    mkdir(path[projectPath], 0777);
    writeLayer(path[filePath], path[layerPath], &chart);
    writeBPM(path[bpmPath], &chart);
    writeEvd(path[evdPath], &chart);
    writePPD(path[ppdPath], path[layerPath], path[divascriptPath], path[csinputPath], path[bpmPath], path[evdPath]);

    if (operation != 2)
    {
        writeIni(path[iniPath], &chart);
        mkdir(path[soundFolderPath], 0777);
        copyTextFile(path[soundPath], path[targetSoundPath], "wb");
        copyTextFile(path[soundsetPath], path[targetSoundsetPath], "wb");

        if (operation == 1)
        {
            mkdir(path[scriptFolderPath], 0777);
            mkdir(path[layerFolderPath], 0777);
            copyTextFile(path[csinputPath], path[targetCsinputPath], "wb");
            copyTextFile(path[divascriptPath], path[targetDivascriptPath], "wb");
            copyTextFile(path[scdPath], path[targetScdPath], "wb");
            writeLayer(path[filePath], path[targetLayerPath], &chart);
            writePpdproj(path[ppdprojPath], path[difficultyChar], &chart);
            writeBPM(path[targetBpmPath], &chart);
        }
    }

    freePath(&path);
    goto end;

error:
    fprintf(stderr, "%s", errmsg);
    fflush(stdin); getchar();
    return 1;

end:
    printf("\nPress Enter to leave\n");
    fflush(stdin); getchar();

    return 0;
}