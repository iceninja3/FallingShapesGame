import pygame # type: ignore
import random
import sys
import serial # type: ignore
import time

#run following command to run game
#python3 fall.py

time.sleep(2)  # Sleep for 2 seconds

expected_message = "Hello\n"
response_message = "I got your message\n"

# Serial Port Setup
SERIAL_PORT = "/dev/tty.usbmodem103"  # Change this to match your system
BAUD_RATE = 9600
serial_connection = None


# Initialize Pygame
pygame.init()

# Screen Dimensions
SCREEN_WIDTH = 400
SCREEN_HEIGHT = 800
FPS = 60

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)

# Shapes
SHAPE_SIZE = 50

# Game State
game_running = False
game_paused = False


try:
    serial_connection = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    print(f"Connected to {SERIAL_PORT}")
except serial.SerialException as e:
    print(f"Error connecting to serial port: {e}")
    sys.exit(1)

# Initialize Pygame Screen
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("Shape Match Game")

# Clock for controlling frame rate
clock = pygame.time.Clock()

# Font
font = pygame.font.Font(None, 36)


class Shape:
    def __init__(self, x, y, shape_type):
        self.x = x
        self.y = y
        self.shape_type = shape_type
        self.color = RED if shape_type == "square" else GREEN if shape_type == "triangle" else BLUE

    def draw(self):
        if self.shape_type == "square":
            pygame.draw.rect(screen, self.color, (self.x, self.y, SHAPE_SIZE, SHAPE_SIZE))
        elif self.shape_type == "triangle":
            points = [(self.x, self.y + SHAPE_SIZE), (self.x + SHAPE_SIZE / 2, self.y), (self.x + SHAPE_SIZE, self.y + SHAPE_SIZE)]
            pygame.draw.polygon(screen, self.color, points)
        elif self.shape_type == "line":
            pygame.draw.line(screen, self.color, (self.x, self.y), (self.x, self.y + SHAPE_SIZE), 5)

    def fall(self, speed):
        self.y += speed


def display_text(text, y_offset=0):
    text_surface = font.render(text, True, WHITE)
    text_rect = text_surface.get_rect(center=(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + y_offset))
    screen.blit(text_surface, text_rect)


def handle_serial_input():
    """Reads input from the serial port and processes commands."""
    global game_running, game_paused
    try:
        if serial_connection.in_waiting > 0:
            command = serial_connection.readline().decode("ascii").strip()
            print(f"Received command: {command}")

            if command == "1!": #START
                game_running = True
                game_paused = False
            elif command == "2!": #PAUSE
                game_paused = True
            elif command == "3!": #END GAME
                pygame.quit()
                sys.exit()
            elif game_running and not game_paused:
                if command == "4!": #SQUARE
                    return "square"
                elif command == "5!": #TRIANGLE
                    return "triangle"
                elif command == "6!": #LINE
                    return "line"
    except Exception as e:
        print(f"Error reading serial input: {e}")
    return None


def main():
    global game_running, game_paused

    shapes = []
    fall_speed = 1
    score = 0
    lives = 1000

    # Main Game Loop
    while True:
        screen.fill(BLACK)

        # Handle Pygame Events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        # Handle Serial Input
        command = handle_serial_input()

        if command:
            shapes = [shape for shape in shapes if shape.shape_type != command]
            score += 1

        # Game Logic
        if game_running and not game_paused:
            # Spawn shapes randomly
            if random.randint(1, 50) == 1:  # Adjust for shape spawn frequency
                x = random.randint(0, SCREEN_WIDTH - SHAPE_SIZE)
                shape_type = random.choice(["square", "triangle", "line"])
                shapes.append(Shape(x, 0, shape_type))

            # Move and draw shapes
            for shape in shapes[:]:
                shape.fall(fall_speed)
                if shape.y > SCREEN_HEIGHT:  # Shape hits the ground
                    shapes.remove(shape)
                    lives -= 1
                    if lives <= 0:
                        game_running = False
                shape.draw()

            # Draw score and lives
            score_text = font.render(f"Score: {score}", True, WHITE)
            lives_text = font.render(f"Lives: {lives}", True, WHITE)
            screen.blit(score_text, (10, 10))
            screen.blit(lives_text, (10, 40))

        # Pause Screen
        if game_paused:
            display_text("Game Paused")

        # Game Over Screen
        if not game_running and not game_paused:
            display_text("Game Over", -50)  # Move "Game Over" up slightly
            display_text(f"Score: {score}", 0)  # Leave "Score" in the center
            display_text("Start game by performing 'START'", 50)  # Add start game prompt below
            #display_text("VISHAL HIGHLY RECCOMENDS PLAYING WITH BOARD ON A VERY BIG SURFACE'", -100)  # Add start game prompt below

        pygame.display.flip()
        clock.tick(FPS)


if __name__ == "__main__":
    main()
