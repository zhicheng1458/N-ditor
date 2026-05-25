This project is meant to be a close replication (but not quite!) of the level editor in [N++](https://store.steampowered.com/app/230270/N_NPLUSPLUS/), with the aim to make access to more advanced level editing technique that would otherwise be unachievable without knowing how to hex edit the level file.

Control: (Require mouse and keyboard)
Everything is (mostly) the same as you would expect in the actual editor!

Minor difference:
There are no grid snapping to entities (for now), this allows for finer placement of entities.
Entity selection are done in a tray. Use scroll key to scroll through the tray to find the entities you would like.
Object rotation can now be additionally controlled by the key Q, E, Z, and C, indicating diagonal direction.
Object mode are selected via the numeric key 1, 2, 3, 4, following the format as given in the [metanet level data](https://i.imgur.com/qjm4AEc.png)

Other control:
L key to import your level from a default file named "Untitled-1" within the level folder in this project.
P key to export your level to a default file named "Untitled-1" within the level folder in this project. Exporting will only track the items within the border in the editor.
Use your mouse scroll key to zoom in and out.
Hold down the right mouse key and drag around to shift the view of the level editor.

Credits:
Library used: GLFW, GLEW, GLM, Dear ImGUI
Entities sprite obtained from [inne](https://github.com/edelkas/inne) by Eddy.
