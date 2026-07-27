package rat

import rl "vendor:raylib"

main :: proc() {
	// raylib speedrun

	rl.InitWindow(240, 320, "hello.")
	defer rl.CloseWindow()

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.BLACK)
		rl.EndDrawing()
	}
}
