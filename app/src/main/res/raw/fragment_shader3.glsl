precision mediump float;
varying vec2 ft_Position;
uniform sampler2D sTexture;
void main() {
    lowp vec4 textureColor = texture2D(sTexture, ft_Position);
    float r = textureColor.r - 0.5;
    float g = textureColor.g - 0.5;
    float b = textureColor.b - 0.5;
    gl_FragColor = vec4(r , g, b, textureColor.w);


}
