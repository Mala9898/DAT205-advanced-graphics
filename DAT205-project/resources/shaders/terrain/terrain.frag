#version 330 core

out vec4 FragColor;

in float Height;

void main()
{
    float h = (Height + 20)/30.0f;	// shift and scale the height in to a grayscale value
//    if (h < 0.5) {
//        FragColor = vec4(0.1, 0.1, 0.1, 1);
//    } else{
//        FragColor = vec4(1, 0, 0, 1);
//    }
    FragColor = vec4(h, h, h, 1.0);
}