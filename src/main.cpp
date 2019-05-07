#include "allheader.h" 
#include "shader.h"
//#include "camera.h"
#include "Vertex.h"

//异常回调函数
static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//顶点着色器
const char *vertexShaderSource1 = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 pers;\n"
"void main()\n"
"{\n"
"	vec4 dot= vec4(aPos.x,aPos.y, aPos.z, 1.0f);\n"	
"   gl_Position = pers*view*model*dot; \n"
"}\0";

//片段着色器
const char *fragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"   FragColor = color;\n"
"}\n\0";


//立方体的六个面->每个面由两个三角形组成，每个面6个顶点

int main(int, char**)
{
	//-------------------------------------------------------------
	// Setup window、设置异常回调函数
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	//-------------------------------------------------------------
	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	

	//-----------------------------------------------------------------------------
	// 声明一个窗口对象

	GLFWwindow* window = glfwCreateWindow(MAXWIDTH,MAXHEIGHT, "OpenGL3", NULL, NULL);
	if (window == NULL)
		return 1;

	//为窗口对象创建一个乡下文
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	//设置回调
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	//---------------------------------------------------------------
	// 初始化opengl函数加载器
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	//是否开启深度测试
	if (ENABLE) glEnable(GL_DEPTH_TEST);

	//-----------------------------------------------------------------
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	//-------------------------------------------------------------------
	//生成着色器程序:从上到下依次为Phone shading\Gouraud shading\Light resource的shader
	shader shaderProgramCubePhong = shader();
	if (!shaderProgramCubePhong.createShaderWithFile("./PhongVS.txt", "./PhongFS.txt")) {
		std::cout << "Phong shader program create failly!" << std::endl;
		exit(1);
	}
	shader shaderProgramCubeGouraud = shader();
	if (!shaderProgramCubeGouraud.createShaderWithFile("./GouraudVS.txt", "./GouraudFS.txt")) {
		std::cout << "Gouraud shader program create failly!" << std::endl;
		exit(1);
	}
	shader shaderProgramLight = shader();
	if (!shaderProgramLight.createShaderWithChar(vertexShaderSource1, fragmentShaderSource1)) {
		std::cout << "Gouraud shader program create failly!" << std::endl;
		exit(1);
	}
	//--------------------------------------------------------------------
	//菜单选项的值
	bool phongShading = false;
	//环境因子，漫反射因子，镜面反射因子
	float Ambient=0.3, Diffuse=0.5, Specular=0.5;
	//反光度
	int Shininess = 2;
	//RGB颜色，用于渲染平面（两个三角形）时的颜色
	float ccolor[3] = { 1.0f,1.0f,1.0f};
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.0);
	glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
	//6对VAO、VBO，每队VAO、VBO负责画立方体的一个面
	unsigned int VAO[2], VBO;
	glGenBuffers(1,&VBO);
	glGenVertexArrays(2, VAO);
	
	//对于直线，绑定VAO、VBO，并将直线的顶点数据写入缓存中
	//cube
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//light source
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Main loop
	while (!glfwWindowShouldClose(window))//判定窗口是否关闭，依次来进行循环渲染
	{
		int all = 0;
		
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		float a = glfwGetTime();
		lightPos.x = sin(a);
		lightPos.z = cos(a);
		//以下是创建一个图形交互界面
		ImGui::Begin("Light and Shading!", NULL, ImGuiWindowFlags_MenuBar);
		ImGui::Checkbox("Phong Shading", &phongShading);
		ImGui::SliderFloat("Ambient Factor", &Ambient, 0.0f, 0.5f);
		ImGui::SliderFloat("Diffuse Factor", &Diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular Factor", &Specular, 0.0f, 1.0f);
		ImGui::SliderInt("Shininess", &Shininess, 2, 256);
		ImGui::ColorPicker3("Light Source Color", (float*)ccolor);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		
		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		//调节视口，让其与窗口大小保存一致
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//清屏，以此来设置背景色
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//每次循环刷新图形界面的帧
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		if (phongShading) { 
			shaderProgramCubePhong.use(); 
			//设定光源颜色
			shaderProgramCubePhong.setVec3("lightColor",glm::vec3(ccolor[0],ccolor[1],ccolor[2]));
			//传入光源位置
			shaderProgramCubePhong.setVec3("lightPos", lightPos);
			//传入观察位置
			shaderProgramCubePhong.setVec3("viewPos", viewPos);
			//传入对象颜色
			shaderProgramCubePhong.setVec3("objectColor",glm::vec3(1.0f, 0.5f, 0.31f));
			//传入环境因子
			shaderProgramCubePhong.setFloat("ambientStrength", Ambient);
			//传入漫反射因子
			shaderProgramCubePhong.setFloat("diffuseStrength", Diffuse);
			//传入镜面反射因子
			shaderProgramCubePhong.setFloat("specularStrength", Specular);
			//传入反光度
			shaderProgramCubePhong.setInt("shininess", Shininess);
		}
		else {
			shaderProgramCubeGouraud.use();
			shaderProgramCubeGouraud.setVec3("lightColor", glm::vec3(ccolor[0], ccolor[1], ccolor[2]));
			shaderProgramCubeGouraud.setVec3("lightPos", lightPos);
			shaderProgramCubeGouraud.setVec3("viewPos", viewPos);
			shaderProgramCubeGouraud.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
			shaderProgramCubeGouraud.setFloat("ambientStrength", Ambient);
			shaderProgramCubeGouraud.setFloat("diffuseStrength", Diffuse);
			shaderProgramCubeGouraud.setFloat("specularStrength", Specular);
			shaderProgramCubeGouraud.setInt("shininess", Shininess);
		}
		//初始化三个矩阵为单位矩阵
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 pers = glm::mat4(1.0f);
		
		//将物体进行两种映射
		//model = glm::rotate(model, glm::radians(10.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		view = glm::translate(view, glm::vec3(0.0f, -0.5f, -6.0f));
		pers = glm::perspective(glm::radians(45.0f), (float)MAXWIDTH / (float)MAXHEIGHT, 0.1f, 100.0f);

		if (phongShading) {
			//shaderProgramCubePhong.use();
			shaderProgramCubePhong.setVertexUniformModel(model,"model");
			//设置view矩阵
			shaderProgramCubePhong.setVertexUniformView(view,"view");
			//设置透视矩阵
			shaderProgramCubePhong.setVertexUniformPers(pers,"projection");
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}//设置模型矩阵
		else {
			shaderProgramCubeGouraud.use();
			shaderProgramCubeGouraud.setVertexUniformModel(model, "model");
			//设置view矩阵
			shaderProgramCubeGouraud.setVertexUniformView(view, "view");
			//设置透视矩阵
			shaderProgramCubeGouraud.setVertexUniformPers(pers, "projection");
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glm::mat4 model1 = glm::mat4(1.0f);
		glm::mat4 view2 = glm::mat4(1.0f);
		model1 = glm::rotate(model1, glm::radians(30.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		model1 = glm::translate(model1, lightPos);
		model1 = glm::scale(model1, glm::vec3(0.2, 0.2, 0.2));
		view2 = glm::translate(view2, glm::vec3(0.0f, -0.5f, -6.0f));
		shaderProgramLight.use();
		shaderProgramLight.setVertexUniformModel(model1, "model");
		shaderProgramLight.setVertexUniformView(view2, "view");
		shaderProgramLight.setVertexUniformPers(pers, "pers");
		shaderProgramLight.setFragmentUniformColor(ImVec4(ccolor[0], ccolor[1], ccolor[2], 1.0f), "color");
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	// Cleanup
	//解除ImGUI与opengl、glfw之间的关联资源
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	//释放imGUI的相关资源
	ImGui::DestroyContext();
	//删除前面所生成的VAO、VBO
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(1, &VBO);
	//释放窗口对象
	glfwDestroyWindow(window);
	//释放所有的窗口资源
	glfwTerminate();

	return 0;
}
