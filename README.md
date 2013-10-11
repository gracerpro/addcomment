#Name
addcomment -- add comment to source code.

#Decsription
This programm allow add the comment to source code. The comments places to begining of file. Between the comments and first byte of source code skips all whitespace.

#Synopsis
addcomment path_to_direcrory [comment_file_name] [filter_str] [-b]

addcomment -dir path_to_direcrory [-c FILE] [-f[ilter] filter_str] [-b]

#Options
* **-dir path_to_direcrory** (The path to target directory, absolute or relative)
* **-c comment_file_name** (The comment file name, absolute or relative)
* **-f filter_str** (string of filter, each filter separated by semicolon, for example "*.cpp;*.h"). Default "*.*".
* **-b** (If this option is set, then will be created backup file)

#Version
addcomment 1.0
