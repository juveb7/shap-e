import argparse
import sys, os, subprocess
import torch
from shap_e.diffusion.sample import sample_latents
from shap_e.diffusion.gaussian_diffusion import diffusion_from_config
from shap_e.models.download import load_model, load_config
from shap_e.util.notebooks import decode_latent_mesh
from shap_e.util.image_util import load_image

def generate_object_file_from_text(prompt, object_file_path, device):
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
        with open(f'{object_file_path}_{i}.obj', 'w') as f:
            t.write_obj(f)

def generate_object_file_from_image(image_path, object_file_path, device):
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
        with open(f'{object_file_path}_{i}.obj', 'w') as f:
            t.write_obj(f)

def run_cpp_program(object_file_paths):
    # Specify the path to the compiled C++ executable
    cpp_executable_path = "/home/918573232/csc630/code/shap-e/shap_e/pipeline/P3.exe"

    # Run the C++ program with the object file path as an argument
    result = subprocess.run([cpp_executable_path, object_file_paths[0], object_file_paths[1], object_file_paths[2]], text=True, capture_output=True)

    # Print stdout and stderr from the C++ program
    print("STDOUT:", result.stdout)
    print("STDERR:", result.stderr)
    print("Return Code:", result.returncode)

def main():
    parser = argparse.ArgumentParser(description="Generate 3D object files from text or image inputs.")
    parser.add_argument('--text', type=str, help="Text prompt for generating 3D objects.")
    parser.add_argument('--image', type=str, help="Image file path for generating 3D objects.")
    parser.add_argument('--output_object_path', type=str, help="Path to object file output directory.")
    parser.add_argument('--output_object_filename', type=str, help="Filename of the output object file.")
    args = parser.parse_args()

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

    output_file_path = os.path.join(args.output_object_path, args.output_object_filename)

    if args.text:
        print("Generating object from text prompt...")
        generate_object_file_from_text(args.text, output_file_path, device)
    elif args.image:
        print("Generating object from image...")
        generate_object_file_from_image(args.image, output_file_path, device)
    else:
        print("No input provided. Please specify either --text or --image.")
        sys.exit(1)

    print("Object file generation complete.")

    object_files = [output_file_path+"_0.obj", output_file_path+"_1.obj", output_file_path+"_2.obj"]
    run_cpp_program(object_files)

if __name__ == "__main__":
    main()
