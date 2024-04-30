import argparse
import sys
import torch
from shap_e.diffusion.sample import sample_latents
from shap_e.diffusion.gaussian_diffusion import diffusion_from_config
from shap_e.models.download import load_model, load_config
from shap_e.util.notebooks import decode_latent_mesh
from shap_e.util.image_util import load_image

def generate_object_file_from_text(prompt, device):
    model = load_model('text300M', device=device)
    diffusion = diffusion_from_config(load_config('diffusion'))
    batch_size = 4
    guidance_scale = 15.0

    latents = sample_latents(
        batch_size=batch_size,
        model=model,
        diffusion=diffusion,
        guidance_scale=guidance_scale,
        model_kwargs=dict(texts=[prompt] * batch_size),
        progress=True,
        clip_denoised=True,
        use_fp16=True,
        use_karras=True,
        karras_steps=64,
        sigma_min=1e-3,
        sigma_max=160,
        s_churn=0,
    )

    # Save latents as object files
    for i, latent in enumerate(latents):
        t = decode_latent_mesh(model, latent).tri_mesh()
        with open(f'example_mesh_{i}.obj', 'w') as f:
            t.write_obj(f)

def generate_object_file_from_image(image_path, device):
    model = load_model('image300M', device=device)
    diffusion = diffusion_from_config(load_config('diffusion'))
    batch_size = 4
    guidance_scale = 3.0
    image = load_image(image_path)

    latents = sample_latents(
        batch_size=batch_size,
        model=model,
        diffusion=diffusion,
        guidance_scale=guidance_scale,
        model_kwargs=dict(images=[image] * batch_size),
        progress=True,
        clip_denoised=True,
        use_fp16=True,
        use_karras=True,
        karras_steps=64,
        sigma_min=1e-3,
        sigma_max=160,
        s_churn=0,
    )

    # Save latents as object files
    for i, latent in enumerate(latents):
        t = decode_latent_mesh(model, latent).tri_mesh()
        with open(f'example_image_mesh_{i}.obj', 'w') as f:
            t.write_obj(f)

def main():
    parser = argparse.ArgumentParser(description="Generate 3D object files from text or image inputs.")
    parser.add_argument('--text', type=str, help="Text prompt for generating 3D objects.")
    parser.add_argument('--image', type=str, help="Image file path for generating 3D objects.")
    args = parser.parse_args()

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

    if args.text:
        print("Generating object from text prompt...")
        generate_object_file_from_text(args.text, device)
    elif args.image:
        print("Generating object from image...")
        generate_object_file_from_image(args.image, device)
    else:
        print("No input provided. Please specify either --text or --image.")
        sys.exit(1)

    print("Object file generation complete.")
    os.system('/P3')

if __name__ == "__main__":
    main()
