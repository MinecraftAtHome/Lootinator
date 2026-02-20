<p align="center">
  <img align="center" src="https://github-production-user-asset-6210df.s3.amazonaws.com/85095943/552607332-e47f3c1e-d22e-40f7-972d-53fced427715.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20260220%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20260220T092341Z&X-Amz-Expires=300&X-Amz-Signature=0b5ea976c71dccd370a23bbe93c29418f366cf7a9742c134f2e7ad330df88bf5&X-Amz-SignedHeaders=host"></img>
</p>
<h1 align="center"><b>Lootinator</b></h1>
<p align="center">
  <img src="https://img.shields.io/github/repo-size/Kludwisz/Lootinator?style=%22for-the-badge%22"></img>
  <img src="https://img.shields.io/github/last-commit/Kludwisz/Lootinator?style=%22for-the-badge%22"></img>
  <img src="https://img.shields.io/github/languages/top/Kludwisz/Lootinator?style=for-the-badge">
</p>
<p align="center">
GPU accelerated Minecraft loot finding
</p>

## About

Lootinator on its own is not a program for finding loot, Lootinator's job is to generate an optimized self-contained CUDA kernel(s) which can either be executed directly or elsewhere. Currently all interactions are done through the API, but the end goal is to support a wide range of frontends, native applications, discord bots, even web interfaces.
```cpp
int main() {
  // config
	kgen::KernelGenConfig kgen_config = {false,
		"../../example/src/simple_constraints_2.json",
		"../../example/src/item_map.txt",
		"../../example/src/ruined_portal.json",
		mc::MC_1_21_TO_1_21_9};
	std::vector<kgen::ConfiguredKernel> kernels;

  // generate all "bruteforce" kernels
	kgen::BruteforceKernel::gen_kernels(kernels, kgen_config);
}
```

## Warning

Lootinator is still an active work in progress, the API could change at any moment and there are still plenty of features missing, development happens in frequent voice chats on the Minecraft@Home discord server (https://discord.gg/xArErFf) to report an issue head over to the Lootinator thread in #general-work 
