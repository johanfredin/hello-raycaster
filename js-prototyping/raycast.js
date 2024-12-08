const TILE_SIZE = 32;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;
const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;

const FOV_ANGLE = degToRad(60);
const WALL_STRIP_WIDTH = 30;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;



function degToRad(degrees) {
    return degrees * (Math.PI / 180);
}

function normalizeAngle(angle) {
    // Clamp angle to be between 0 and 2PI (0-360 degrees)
    angle = angle % (Math.PI * 2);
    if (angle < 0) {
        angle = (Math.PI * 2) + angle;
    }
    return angle;
}

class Map {
    constructor() {
        this.grid = [
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        ];
    }
    render() {
        for (let i = 0; i < MAP_NUM_ROWS; i++) {
            for (let j = 0; j < MAP_NUM_COLS; j++) {
                let tileX = j * TILE_SIZE;
                let tileY = i * TILE_SIZE;
                let tileColor = this.grid[i][j] == 1 ? "#222" : "#fff";
                stroke("#222");
                fill(tileColor);
                rect(tileX, tileY, TILE_SIZE, TILE_SIZE);
            }
        }
    }

    hasWallAt(x, y) {
        if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
            return true;
        }

        const gridX = Math.floor(x / TILE_SIZE);
        const gridY = Math.floor(y / TILE_SIZE);
        return this.grid[gridY][gridX] == 1;
    }
}

class Player {
    constructor() {
        this.x = WINDOW_WIDTH / 2;
        this.y = WINDOW_HEIGHT / 2;
        this.radius = 3;        // Size of player
        this.turnDirection = 0; // -1 if left or +1 if right
        this.walkDirection = 0; // -1 if back of +1 if front
        this.rotationAngle = Math.PI / 2;
        this.moveSpeed = 2.0;
        this.rotationSpeed = 2 * (Math.PI / 180);
    }

    render() {
        noStroke();
        fill("red");
        circle(this.x, this.y, this.radius);
        stroke("red");
        line(
            this.x,
            this.y,
            this.x + Math.cos(this.rotationAngle) * 20,
            this.y + Math.sin(this.rotationAngle) * 20,
        )
    }

    update() {
        this.rotationAngle += this.turnDirection * this.rotationSpeed;
        const moveStep = this.walkDirection * this.moveSpeed;

        // Calculate what next x and y will be 
        const newPlayerX = this.x + Math.cos(this.rotationAngle) * moveStep;
        const newPlayerY = this.y + Math.sin(this.rotationAngle) * moveStep;

        // Update position if we are not colliding with a wall
        if (!grid.hasWallAt(newPlayerX, newPlayerY)) {
            this.x = newPlayerX;
            this.y = newPlayerY;
        }
    }
}

class Ray {
    constructor(rayAngle) {
        this.rayAngle = normalizeAngle(rayAngle);
        this.wallHitX = 0;
        this.wallHitY = 0;
        this.distance = 0;

        this.isRayFacingDown = this.rayAngle > 0 && this.rayAngle < Math.PI;
        this.isRayFacingUp = !this.isRayFacingDown;
        this.isRayFacingRight = this.rayAngle < (Math.PI / 2) || this.rayAngle > 1.5 * Math.PI;
        this.isRayFacinLeft = !this.isRayFacingRight;
    }

    cast(columnId) {
        /*
        ***************************************
        * Horizontal ray grid intersection code
        ****************************************
        */

        // Find the y coordinate of the closest horisontal grid intersection
        let yintercept = Math.floor(player.y / TILE_SIZE) * TILE_SIZE; 
        if (this.isRayFacingDown) {
            yintercept += TILE_SIZE;
        }
        // Find the x coordinate of the closest horisontal grid intersection
        const xintercept = player.x + (yintercept - player.y) / Math.tan(this.rayAngle);

        // Calculate the increment for xstep and ystep
        let yStep = TILE_SIZE;
        if (this.isRayFacingUp) {
            yStep = -yStep;
        }
        let xStep = TILE_SIZE / Math.tan(this.rayAngle);  
        if (this.isRayFacinLeft && xStep > 0) {
            xStep = -xStep;
        } if (this.isRayFacingRight && xStep < 0) {
            xStep = -xStep;
        }

        let nextHorzTouchX = xintercept;
        let nextHorzTouchY = yintercept;

        if (this.isRayFacingUp) {
            nextHorzTouchY--;
        }

        let foundHorzWallHit = false;
        let wallHitX = 0;
        let wallHitY = 0;

        // Increment xstep and ystep until we find a wall
        const withinBounds = 
            nextHorzTouchX >= 0 && 
            nextHorzTouchX <= WINDOW_WIDTH &&
            nextHorzTouchY >= 0 &&
            nextHorzTouchY <= WINDOW_HEIGHT;  

        while (withinBounds) {
            if (grid.hasWallAt(nextHorzTouchX, nextHorzTouchY)) {
                foundHorzWallHit = true;
                wallHitX = nextHorzTouchX;
                wallHitY = nextHorzTouchY;
                break;
            } else {
                nextHorzTouchX += xStep;
                nextHorzTouchY += yStep;
            }
        }
    }

    render() {
        stroke("rgba(255, 0, 0, 0.66)");
        line(
            player.x,
            player.y,
            player.x + Math.cos(this.rayAngle) * 30,
            player.y + Math.sin(this.rayAngle) * 30
        );
    }
}

const grid = new Map();
const player = new Player();
let rays = [];

function keyPressed() {
    if (keyCode == UP_ARROW) {
        player.walkDirection = 1;
    } else if (keyCode == DOWN_ARROW) {
        player.walkDirection = -1;
    }

    if (keyCode == RIGHT_ARROW) {
        player.turnDirection = 1;
    } else if (keyCode == LEFT_ARROW) {
        player.turnDirection = -1;
    }
}

function keyReleased() {
    if (keyCode == UP_ARROW) {
        player.walkDirection = 0;
    } else if (keyCode == DOWN_ARROW) {
        player.walkDirection = 0;
    }

    if (keyCode == RIGHT_ARROW) {
        player.turnDirection = 0;
    } else if (keyCode == LEFT_ARROW) {
        player.turnDirection = 0;
    }
}

function setup() {
    createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

function castAllRays() {
    let columnId = 0;

    // Start left-most ray by subtracting half of the FOV
    let rayAngle = player.rotationAngle - (FOV_ANGLE / 2);

    rays = [];
    for (let i = 0; i < 1; i++) {
        const ray = new Ray(rayAngle);
        ray.cast(columnId);
        rays.push(ray);

        rayAngle += FOV_ANGLE / NUM_RAYS;
        columnId++;
    }
}

function update() {
    player.update();
    castAllRays();
}

function draw() {
    update();

    grid.render();
    for (let ray of rays) {
        ray.render();
    }
    player.render();
}
