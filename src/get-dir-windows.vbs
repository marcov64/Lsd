' *************************************************************
'
'  LSD 8.0 - May 2022
'  written by Marco Valente, Universita' dell'Aquila
'  and by Marcelo Pereira, University of Campinas
'
'  Copyright Marco Valente and Marcelo Pereira
'  LSD is distributed under the GNU General Public License
'
' See Readme.txt for copyright information of
' third parties' code used in LSD
'
' *************************************************************

' *************************************************************
'  GET-DIR-WINDOWS.VBS
'  Get the path of windows predefined directories.
'  Possible value for the single argument:
'    AllUsersDesktop
'    AllUsersStartMenu
'    AllUsersPrograms
'    AllUsersStartup
'    Desktop
'    Favorites
'    Fonts
'    MyDocuments
'    NetHood
'    PrintHood
'    Programs
'    Recent
'    SendTo
'    StartMenu
'    Startup
'    Templates
' *************************************************************

Dim WshShell
Dim strPath

set WshShell = WScript.CreateObject( "WScript.Shell" )
strPath = WshShell.SpecialFolders( WScript.Arguments( 0 ) )
WScript.echo strPath