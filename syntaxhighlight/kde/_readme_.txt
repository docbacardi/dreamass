DreamAss Syntaxhighlighting for KDE editor components.


Install

Install this in your katepart syntax directory and all applications like KWrite, Kate, KDevelop will display DreamAss sources with highlighting.
On my system the katepart syntax directory for the current user is located here:

~/.kde/share/apps/katepart/syntax

If you never installed syntax highlighting files before, the folder might not exist yet. Just create it by hand and kate will recognize it after a restart.



Conflicts with pic asm highlighting

For some strange reasons the pic asm highlighting definition grabs all fileextensions which look like assembly sources, including the '*.src' for the dreamass highlighting.
Please increase the priority for the dreamass highlighting rules in the Kate config menu ( Config -> Editor -> Highlighting, then select "Assembler/Dreamass").
--
Doc Bacardi/DRM
