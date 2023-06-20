#version 450 core

uniform sampler3D voxelTexture;
uniform sampler2D backFaceTexture;
uniform sampler2D frontFaceTexture;
uniform int mipMapLevel;

in vec2 TexCoords;

out vec4 color;

void main()
{
	vec4 accumulated_color = vec4(0,0,0,0);
	vec3 ray_origin = texture(frontFaceTexture, TexCoords).xyz;
	vec3 ray_end = texture(backFaceTexture, TexCoords).xyz;
	vec3 ray_direction = normalize(ray_end - ray_origin);

	const float ray_step_size = 0.003f;
	int total_samples = int(length(ray_end - ray_origin) / ray_step_size);

	for (int i=0; i < total_samples; i++)
	{
		vec3 sample_location = (ray_origin + ray_direction * ray_step_size * i);
		vec4 texSample = textureLod(voxelTexture, (sample_location + vec3(1.0f)) * 0.5f, mipMapLevel);	

		if (texSample.a > 0) {
			texSample.rgb /= texSample.a;
			accumulated_color.rgb = accumulated_color.rgb + (1.0f - accumulated_color.a) * texSample.a * texSample.rgb;
			accumulated_color.a   = accumulated_color.a   + (1.0f - accumulated_color.a) * texSample.a;
		}

		if (accumulated_color.a > 0.95) // early exit
			break;
	}

	accumulated_color.rgb = pow(accumulated_color.rgb, vec3(1.0f / 2.2f));
	color = accumulated_color;
}
