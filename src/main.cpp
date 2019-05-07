#include "allheader.h" 
#include "shader.h"
//#include "camera.h"
#include "Vertex.h"

//�쳣�ص�����
static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//������ɫ��
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

//Ƭ����ɫ��
const char *fragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"   FragColor = color;\n"
"}\n\0";


//�������������->ÿ������������������ɣ�ÿ����6������

int main(int, char**)
{
	//-------------------------------------------------------------
	// Setup window�������쳣�ص�����
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
	// ����һ�����ڶ���

	GLFWwindow* window = glfwCreateWindow(MAXWIDTH,MAXHEIGHT, "OpenGL3", NULL, NULL);
	if (window == NULL)
		return 1;

	//Ϊ���ڶ��󴴽�һ��������
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	//���ûص�
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);

	//---------------------------------------------------------------
	// ��ʼ��opengl����������
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

	//�Ƿ�����Ȳ���
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
	//������ɫ������:���ϵ�������ΪPhone shading\Gouraud shading\Light resource��shader
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
	//�˵�ѡ���ֵ
	bool phongShading = false;
	//�������ӣ����������ӣ����淴������
	float Ambient=0.3, Diffuse=0.5, Specular=0.5;
	//�����
	int Shininess = 2;
	//RGB��ɫ��������Ⱦƽ�棨���������Σ�ʱ����ɫ
	float ccolor[3] = { 1.0f,1.0f,1.0f};
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.0);
	glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
	//6��VAO��VBO��ÿ��VAO��VBO�����������һ����
	unsigned int VAO[2], VBO;
	glGenBuffers(1,&VBO);
	glGenVertexArrays(2, VAO);
	
	//����ֱ�ߣ���VAO��VBO������ֱ�ߵĶ�������д�뻺����
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
	while (!glfwWindowShouldClose(window))//�ж������Ƿ�رգ�����������ѭ����Ⱦ
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
		//�����Ǵ���һ��ͼ�ν�������
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
		//�����ӿڣ������봰�ڴ�С����һ��
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//�������Դ������ñ���ɫ
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//ÿ��ѭ��ˢ��ͼ�ν����֡
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		if (phongShading) { 
			shaderProgramCubePhong.use(); 
			//�趨��Դ��ɫ
			shaderProgramCubePhong.setVec3("lightColor",glm::vec3(ccolor[0],ccolor[1],ccolor[2]));
			//�����Դλ��
			shaderProgramCubePhong.setVec3("lightPos", lightPos);
			//����۲�λ��
			shaderProgramCubePhong.setVec3("viewPos", viewPos);
			//���������ɫ
			shaderProgramCubePhong.setVec3("objectColor",glm::vec3(1.0f, 0.5f, 0.31f));
			//���뻷������
			shaderProgramCubePhong.setFloat("ambientStrength", Ambient);
			//��������������
			shaderProgramCubePhong.setFloat("diffuseStrength", Diffuse);
			//���뾵�淴������
			shaderProgramCubePhong.setFloat("specularStrength", Specular);
			//���뷴���
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
		//��ʼ����������Ϊ��λ����
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 pers = glm::mat4(1.0f);
		
		//�������������ӳ��
		//model = glm::rotate(model, glm::radians(10.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		view = glm::translate(view, glm::vec3(0.0f, -0.5f, -6.0f));
		pers = glm::perspective(glm::radians(45.0f), (float)MAXWIDTH / (float)MAXHEIGHT, 0.1f, 100.0f);

		if (phongShading) {
			//shaderProgramCubePhong.use();
			shaderProgramCubePhong.setVertexUniformModel(model,"model");
			//����view����
			shaderProgramCubePhong.setVertexUniformView(view,"view");
			//����͸�Ӿ���
			shaderProgramCubePhong.setVertexUniformPers(pers,"projection");
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}//����ģ�;���
		else {
			shaderProgramCubeGouraud.use();
			shaderProgramCubeGouraud.setVertexUniformModel(model, "model");
			//����view����
			shaderProgramCubeGouraud.setVertexUniformView(view, "view");
			//����͸�Ӿ���
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
	//���ImGUI��opengl��glfw֮��Ĺ�����Դ
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	//�ͷ�imGUI�������Դ
	ImGui::DestroyContext();
	//ɾ��ǰ�������ɵ�VAO��VBO
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(1, &VBO);
	//�ͷŴ��ڶ���
	glfwDestroyWindow(window);
	//�ͷ����еĴ�����Դ
	glfwTerminate();

	return 0;
}
