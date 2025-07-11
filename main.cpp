#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <iostream>
#include <cstdint>
#include <string>
#include <functional>
#include <algorithm>
#include <vector>
#include <cmath>


Image quantizeImg(Image img, std::function<Color(Color)> QuantizeFunc) {
    Color* pixels = LoadImageColors(img);

    int width = img.width, height = img.height;

    Color* quantizedPixels = (Color*)malloc(width*height*sizeof(Color));
    memcpy(quantizedPixels, pixels, width*height*sizeof(Color));
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            quantizedPixels[row*width+col] = QuantizeFunc(quantizedPixels[row*width+col]);
        }
    }

    Image quantized1BitImg = {
        .data = quantizedPixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    UnloadImageColors(pixels);
    return quantized1BitImg;
}

Image convertRGBImgToGrayscale(Image img, std::function<Color(Color)> convertRGBPixToGrayscale) {
    Color* pixels = LoadImageColors(img);

    int width = img.width, height = img.height;

    Color* grayPixels = (Color*)malloc(width*height*sizeof(Color));
    memcpy(grayPixels, pixels, width*height*sizeof(Color));
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            grayPixels[row*width+col] = convertRGBPixToGrayscale(grayPixels[row*width+col]);
        }
    }

    Image grayscaleImg = {
        .data = grayPixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    UnloadImageColors(pixels);
    return grayscaleImg;
}

std::vector<std::vector<Color>> convertImgTo2dArrayPixelVals(Image img) {
    Color* pixels = LoadImageColors(img);
    int height = img.height, width = img.width;
    std::vector<std::vector<Color>> pixels_2d(height, std::vector<Color>(width));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pixels_2d[i][j] = pixels[i*width+j];
        }
    }
    UnloadImageColors(pixels);
    return pixels_2d;
}

Image Dither_FloydSteinberg(Image img, std::function<Color(Color)> funcQuantize) {
    auto pixels = convertImgTo2dArrayPixelVals(img);
    int height = img.height, width = img.width;

    Color* ditheredPixels = (Color*)malloc(height*width*sizeof(Color));

    // Initialize ditheredPixels with original image data
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ditheredPixels[i*width+j] = pixels[i][j];
        }
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Color op = ditheredPixels[row*width+col];
            Color qp = funcQuantize(op);

            int32_t error[3] = {
                op.r - qp.r,
                op.g - qp.g,
                op.b - qp.b
            };
            ditheredPixels[row*width+col] = qp;

            auto updatePixel = [&](const std::vector<int> &dirs, const float errorFactor) {
                int nRow = row + dirs[0], nCol = col + dirs[1];
                if (nRow >= 0 && nRow < height && nCol >= 0 && nCol < width) {
                    int pos = nRow * width + nCol;
                    Color pix = ditheredPixels[pos];
                    int32_t temp[3] = { pix.r, pix.g, pix.b };
                    temp[0] += int32_t(float(error[0]) * errorFactor);
                    temp[1] += int32_t(float(error[1]) * errorFactor);
                    temp[2] += int32_t(float(error[2]) * errorFactor);
                    
                    const unsigned char rgb[3] = { 
                        (unsigned char)std::clamp(temp[0], (int32_t)0, (int32_t)255), 
                        (unsigned char)std::clamp(temp[1], (int32_t)0, (int32_t)255), 
                        (unsigned char)std::clamp(temp[2], (int32_t)0, (int32_t)255) 
                    };
                    ditheredPixels[pos] = (Color){ rgb[0], rgb[1], rgb[2], pix.a };
                }
            };
            // Correct Floyd-Steinberg error distribution pattern
            updatePixel({ 0, 1 }, 7.0f/16.0f);   // Right
            updatePixel({ 1, -1 }, 3.0f/16.0f);  // Below-left  
            updatePixel({ 1, 0 }, 5.0f/16.0f);   // Below
            updatePixel({ 1, 1 }, 1.0f/16.0f);   // Below-right
        }
    }

    Image ditheredImage = {
        .data = ditheredPixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return ditheredImage;
}


int main()
{
    // const int monitor = GetCurrentMonitor();
    // const int screenW = GetMonitorWidth(monitor) * 0.9;
    // const int screenH = GetMonitorHeight(monitor) * 0.9;
    int screenW = 1600;
    int screenH = 800;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);  // Handle high DPI displays properly
    InitWindow(screenW, screenH, "A basic zoomable and panable Window");
    
    // // Debug: Check actual window size
    // int actualWidth = GetScreenWidth();
    // int actualHeight = GetScreenHeight();
    // std::cout << "Requested: " << screenW << "x" << screenH << std::endl;
    // std::cout << "Actual: " << actualWidth << "x" << actualHeight << std::endl;
    
    // Use actual window size for UI layout
    Rectangle settingsPanel = { 0.02f*screenW, 0.02f*screenH, 0.2f*screenW, 0.95f*screenH };
    Rectangle viewport = { 0.23f*screenW, 0.02f*screenH, 0.7f*screenW, 0.95f*screenH };
    
    int settingsInstructX = settingsPanel.x + 30;
    Camera2D camera = {0};
    camera.zoom = 1.0f;

    // Convert RGB Image to Grayscale
    auto convertRGBPixToGrayscale = [](const Color pix) {
        const unsigned char grayscale = (0.2162f * float(pix.r)) + (0.7152f * float(pix.g)) + (0.0722 * float(pix.b));
        // uint8_t a = pix.a;
        return Color{ grayscale, grayscale, grayscale, pix.a };
    };

    // Quantize Grayscale Image to 1-Bit (BLACK and WHITE)
    auto Quantize_Grayscale_1Bit = [](const Color pix) {
        return pix.r < 128 ? BLACK : WHITE;
    };

    // Quantize Grayscale Image to N-Bits 
    int quantizationBits = 2;  // Default to 2 bits
    auto Quantize_Grayscale_NBit = [&quantizationBits](const Color pix) {
        const float varCols = (1 << quantizationBits) - 1;
        const unsigned char col = (unsigned char)(std::clamp(std::round((float(pix.r) / 255.0f) * varCols) / varCols * 255.0f, 0.0f, 255.0f));
        return Color{ col, col, col, pix.a };
    };

    // Quantize RGB Image to N-Bits
    auto Quantize_RGB_NBit = [&quantizationBits](const Color pix) {
        const float varCols = (1 << quantizationBits) - 1;

        const unsigned char colR = (unsigned char)(std::clamp(std::round((float(pix.r) / 255.0f) * varCols) / varCols * 255.0f, 0.0f, 255.0f));
        const unsigned char colG = (unsigned char)(std::clamp(std::round((float(pix.g) / 255.0f) * varCols) / varCols * 255.0f, 0.0f, 255.0f));
        const unsigned char colB = (unsigned char)(std::clamp(std::round((float(pix.b) / 255.0f) * varCols) / varCols * 255.0f, 0.0f, 255.0f));
        return Color { colR, colG, colB, pix.a };
    };

    // Quantize image based on custom palette
    // -------------------------------------------------------------
    /* Change the color palette by modifying the colors in nShades array */
    // -------------------------------------------------------------
    auto Quantize_RGB_CustomPalette = [](const Color pix) {
        Color nShades[5] = { BLACK, WHITE, YELLOW, MAGENTA, Color{ 0, 255, 255, 255 } }; // the last color is CYAN, for some reason it's giving an error using it directly
        float fClosest = INFINITY;
        Color pClosest;

        for (const auto& c : nShades) {
            float fDistance = float(
                std::sqrt(
                    std::pow(float(c.r) - float(pix.r), 2) +
                    std::pow(float(c.g) - float(pix.g), 2) +
                    std::pow(float(c.b) - float(pix.b), 2)));

            if (fDistance < fClosest) {
                fClosest = fDistance;
                pClosest = c;
            }
        }
						
        return pClosest;
    };
    bool customPaletteQuantization = true;

    Image img = LoadImage("resources/nature1.png");
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture2D img_texture = LoadTextureFromImage(img);

    // i know there's a function for this in raylib, but i'm trying to implement stuff myself, so let it be.
    Image grayscaleImg = convertRGBImgToGrayscale(img, convertRGBPixToGrayscale);
    bool convToGrayscale = true;

    Image quantizedImage = quantizeImg(img, Quantize_RGB_NBit);
    bool quantize = true;

    Image ditheredImage = Dither_FloydSteinberg(img, Quantize_RGB_NBit);
    bool ditherQuantizedImage = true;
    
    bool needRegenerate = false;  // Flag to regenerate images when bits change

    // int zoomMode = 0;
    SetTargetFPS(60);


    while (!WindowShouldClose()) {
        int currFPS = GetFPS();
        Vector2 mousePos = GetMousePosition();
        bool mouseInViewport = CheckCollisionPointRec(mousePos, viewport);
        // -------------------------------------------------------------------------------------------------
        // Update 
        // -------------------------------------------------------------------------------------------------
        
        // Handle number key input for quantization bits
        if (IsKeyPressed(KEY_ONE)) { quantizationBits = 1; needRegenerate = true; }
        if (IsKeyPressed(KEY_TWO)) { quantizationBits = 2; needRegenerate = true; }
        if (IsKeyPressed(KEY_THREE)) { quantizationBits = 3; needRegenerate = true; }
        if (IsKeyPressed(KEY_FOUR)) { quantizationBits = 4; needRegenerate = true; }
        if (IsKeyPressed(KEY_FIVE)) { quantizationBits = 5; needRegenerate = true; }
        if (IsKeyPressed(KEY_SIX)) { quantizationBits = 6; needRegenerate = true; }
        if (IsKeyPressed(KEY_SEVEN)) { quantizationBits = 7; needRegenerate = true; }
        if (IsKeyPressed(KEY_EIGHT)) { quantizationBits = 8; needRegenerate = true; }

        if (IsKeyPressed(KEY_R)) {
            camera.zoom = 1.0f;
            camera.offset = {viewport.x, viewport.y};
            camera.target = camera.offset;
        }

        // Regenerate images if quantization bits changed
        if (needRegenerate) {
            UnloadImage(quantizedImage);
            UnloadImage(ditheredImage);
            
            quantizedImage = quantizeImg(img, Quantize_RGB_NBit);
            ditheredImage = Dither_FloydSteinberg(img, Quantize_RGB_NBit);
            needRegenerate = false;
        }
        
        if (IsKeyPressed(KEY_G)) {
            UnloadTexture(img_texture);
            if (convToGrayscale) {
                img_texture = LoadTextureFromImage(grayscaleImg);
            }
            else {
                img_texture = LoadTextureFromImage(img);
            }
            convToGrayscale = !(convToGrayscale);
        }
        if (IsKeyPressed(KEY_Q)) {
            UnloadTexture(img_texture);
            if (quantize) {
                img_texture = LoadTextureFromImage(quantizedImage);
            }
            else {
                img_texture = LoadTextureFromImage(img);
            }
            quantize = !(quantize);
        }
        if (IsKeyPressed(KEY_D)) {
            UnloadTexture(img_texture);
            if (ditherQuantizedImage) {
                img_texture = LoadTextureFromImage(ditheredImage);
            }
            else {
                img_texture = LoadTextureFromImage(img);
            }
            ditherQuantizedImage = !(ditherQuantizedImage);
        }
        if (IsKeyPressed(KEY_C)) {
            if (customPaletteQuantization) {
                UnloadImage(quantizedImage);
                UnloadImage(ditheredImage);

                quantizedImage = quantizeImg(img, Quantize_RGB_CustomPalette);
                ditheredImage = Dither_FloydSteinberg(img, Quantize_RGB_CustomPalette);
            }
            else {
                UnloadImage(quantizedImage);
                UnloadImage(ditheredImage);

                quantizedImage = quantizeImg(img, Quantize_RGB_NBit);
                ditheredImage = Dither_FloydSteinberg(img, Quantize_RGB_NBit);
            }
            customPaletteQuantization = !(customPaletteQuantization);
        }

        if (mouseInViewport) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f/camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                // Vector2 viewportMousePos = { mousePos.x - viewport.x, mousePos.y - viewport.y };
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

                camera.offset = GetMousePosition();
                camera.target = mouseWorldPos;

                float scale = 0.2f*wheel;
                camera.zoom = Clamp(expf(logf(camera.zoom)+scale), 0.125f, 64.0f);
            }
        }

        // -------------------------------------------------------------------------------------------------
        // Draw
        // -------------------------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKGRAY);

            // draw the settings panel
            DrawRectangleRec(settingsPanel, BLACK);
            DrawRectangleLinesEx(settingsPanel, 2, BLUE);


            const char* currFPS_str = ("Current FPS: "+ std::to_string(currFPS)).c_str();
            DrawText(currFPS_str, settingsInstructX, settingsPanel.y+30, 20, MAROON);

            DrawText("Press 'R' to reposition ", settingsInstructX, settingsPanel.y+80, 20, MAROON);
            DrawText("the camera.", settingsInstructX, settingsPanel.y+110, 20, MAROON);

            DrawText("Press 'G' to toggle between ", settingsInstructX, settingsPanel.y+160, 20, MAROON);
            DrawText("grayscale and RGB.", settingsInstructX, settingsPanel.y+190, 20, MAROON);

            DrawText("Press 'Q' while image is in ", settingsInstructX, settingsPanel.y+240, 20, MAROON);
            DrawText("grayscale to toogle between ", settingsInstructX, settingsPanel.y+270, 20, MAROON);
            DrawText("quantized 1-bit image and grayscale.", settingsInstructX, settingsPanel.y+300, 20, MAROON);

            DrawText("Press 'D' while image is in ", settingsInstructX, settingsPanel.y+350, 20, MAROON);
            DrawText("quantized state to toggle between ", settingsInstructX, settingsPanel.y+380, 20, MAROON);
            DrawText("dithered image and grayscale.", settingsInstructX, settingsPanel.y+410, 20, MAROON);
            
            DrawText("Press 1-8 to set quantization bits:", settingsInstructX, settingsPanel.y+460, 20, MAROON);
            std::string bitsText = "Current bits: " + std::to_string(quantizationBits);
            DrawText(bitsText.c_str(), settingsInstructX, settingsPanel.y+490, 20, GREEN);

            DrawText("Press 'C' to toggle between ", settingsInstructX, settingsPanel.y+540, 20, MAROON);
            DrawText("Custom and general color palette.", settingsInstructX, settingsPanel.y+560, 20, MAROON);
            std::string usingCustomColorPalette = "Using custom palette: ";
            usingCustomColorPalette += (customPaletteQuantization == false ? "YES" : "NO");
            DrawText(usingCustomColorPalette.c_str(), settingsInstructX, settingsPanel.y+580, 20, GREEN);

            // draw the viewport
            DrawRectangleRec(viewport, BLACK);

            BeginScissorMode((int)(viewport.x), (int)(viewport.y), (int)(viewport.width), (int)(viewport.height));
            
            BeginMode2D(camera);
                DrawTexture(img_texture, viewport.x, viewport.y, WHITE);
            EndMode2D();

            if (mouseInViewport) {
                DrawCircleV(GetMousePosition(), 4, GRAY);
                DrawTextEx(GetFontDefault(), TextFormat("[%i, %i]", GetMouseX(), GetMouseY()), Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, WHITE);
            }

            DrawRectangleLinesEx(viewport, 2, GREEN);

            EndScissorMode();

        EndDrawing();
    }

    UnloadTexture(img_texture);

    UnloadImage(grayscaleImg);
    UnloadImage(quantizedImage);
    UnloadImage(ditheredImage);
    UnloadImage(img);

    CloseWindow();
    return 0;
}