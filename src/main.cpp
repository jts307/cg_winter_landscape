//#####################################################################
// Main
// Dartmouth COSC 77/177 Computer Graphics, starter code
// Contact: Bo Zhu (bo.zhu@dartmouth.edu)
//#####################################################################
// Editted by Jacob Werzinsky 
//#####################################################################

#include <iostream>
#include <random>
#include <vector>
#include <tuple>
#include <algorithm>
#include <unordered_set>
#include "Common.h"
#include "Driver.h"
#include "OpenGLMesh.h"
#include "OpenGLCommon.h"
#include "OpenGLWindow.h"
#include "OpenGLViewer.h"
#include "OpenGLMarkerObjects.h"
#include "TinyObjLoader.h"
#include <math.h>
#include <algorithm>    
#include <random> 

#ifndef __Main_cpp__
#define __Main_cpp__

#ifdef __APPLE__
#define CLOCKS_PER_SEC 100000
#endif

const float PI = 3.141592653589793f;


class FinalProjectDriver : public Driver, public OpenGLViewer
{using Base=Driver;
	std::vector<OpenGLTriangleMesh*> mesh_object_array;	////mesh objects, every object you put in this array will be rendered.
	clock_t startTime;
	OpenGLScreenCover* screen_cover = nullptr;
	OpenGLBackground* opengl_background = nullptr;

public:
	virtual void Initialize()
	{
		draw_bk=false;		////turn off the default background and use the customized one
		draw_axes = false;	////if you don't like the axes, turn them off!
		startTime=clock();
		OpenGLViewer::Initialize();
	}

	void Add_Shaders()
	{
		////format: vertex shader name, fragment shader name, shader name
		OpenGLShaderLibrary::Instance()->Add_Shader_From_File("background.vert","background.frag","background");	
		OpenGLShaderLibrary::Instance()->Add_Shader_From_File("leaves.vert","leaves.frag","leaves");		
		OpenGLShaderLibrary::Instance()->Add_Shader_From_File("tree_bark.vert","tree_bark.frag","tree_bark");	
		OpenGLShaderLibrary::Instance()->Add_Shader_From_File("fox.vert", "fox.frag", "fox_map");
		OpenGLShaderLibrary::Instance()->Add_Shader_From_File("snow_terrain.vert", "snow_terrain.frag", "snow");
	}

	void Add_Textures()
	{
		////format: image name, texture name
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("maple_albedo.png", "leaves_albedo");		
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("maple_normal.jpg", "leaves_normal");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("bark_albedo.jpg", "bark_albedo");		
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("bark_normal.jpg", "bark_normal");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("snow_albedo.jpg", "plane_albedo");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("snow_normal.jpg", "plane_normal");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("night.jpg", "background_night");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("fox.jpg", "fox_albedo");
		OpenGLTextureLibrary::Instance()->Add_Texture_From_File("fox_normal.jpg", "fox_normal");
	}

	void Add_Background()
	{
		opengl_background=Add_Interactive_Object<OpenGLBackground>();
		opengl_background->shader_name="background";

		////set up texture
		opengl_background->Add_Texture("tex_albedo", OpenGLTextureLibrary::Get_Texture("background_night"));
		Set_Polygon_Mode(opengl_background, PolygonMode::Fill);
		Set_Shading_Mode(opengl_background, ShadingMode::Texture);
		opengl_background->Set_Data_Refreshed();
		opengl_background->Initialize();
	}

	// creates uvs for a given vertex list based on spherical coordinates
	void Update_Uv_Using_Spherical_Coordinates(const std::vector<Vector3>& vertices, std::vector<Vector2>& uv)
	{
		// loop through vertices
		for (int vtx = 0; vtx < vertices.size(); vtx++) {

			// spherical coordinates for a vertex
			float radius = (float)sqrt(vertices[vtx][0] * vertices[vtx][0] + vertices[vtx][1] * vertices[vtx][1] + vertices[vtx][2] * vertices[vtx][2]);
			float theta = (float)acos(vertices[vtx][1] / radius);
			float phi = (float)atan(vertices[vtx][0], vertices[vtx][2]);

			// mapping to [0,1]^2, note: adding 0.5 to u to change where zig-zag artifact shows on sphere
			uv.push_back(Vector2((phi / (2 * PI) + 0.5), (theta / (PI))));
		}
	}

	////function used to add the ground, taken from assignment 4
	int Add_Obj_Mesh_Object(std::string obj_file_name)
	{
		auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();

		Array<std::shared_ptr<TriangleMesh<3> > > meshes;
		Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name, meshes);
		mesh_obj->mesh = *meshes[0];
		std::cout << "load tri_mesh from obj file, #vtx: " << mesh_obj->mesh.Vertices().size() << ", #ele: " << mesh_obj->mesh.Elements().size() << std::endl;

		std::vector<Vector3>& vtx = mesh_obj->mesh.Vertices();
		std::vector<Vector3i>& tri = mesh_obj->mesh.Elements();

		//// Making the plane bigger 
		int vn = (int)vtx.size();
		for (int i = 0; i < vn; i++) {
			vtx[i] = Vector3(100.*(vtx[i][0] - 2.), 100.*(vtx[i][1] - 2.), vtx[i][2]);
		}

		mesh_object_array.push_back(mesh_obj);
		return (int)mesh_object_array.size() - 1;
	}

	//// adding the Fox mesh and editting its vertices
	int Add_Fox()
	{
		auto mesh_obj=Add_Interactive_Object<OpenGLTriangleMesh>();

		////read mesh file
		std::string obj_file_name="fox.obj";
		Array<std::shared_ptr<TriangleMesh<3> > > meshes;
		Obj::Read_From_Obj_File_Discrete_Triangles(obj_file_name,meshes);
		mesh_obj->mesh=*meshes[0];
		
		// shrinking, rotating and moving mesh
		std::vector<Vector3>& vtx = mesh_obj->mesh.Vertices();
		int vn=(int)vtx.size();
		for(int i=0;i<vn;i++){
			// scale
			vtx[i]=Vector3(vtx[i][0]*.003, vtx[i][1]*.003, vtx[i][2]*.003);

			// rotate
			float angle = PI / 2;
			vtx[i]= Vector3(vtx[i][0], vtx[i][1]*cos(angle)-vtx[i][2]*sin(angle), vtx[i][1] * sin(angle) + vtx[i][2] * cos(angle));
			angle = 5*PI/12;
			vtx[i] = Vector3(vtx[i][0] * cos(angle) - vtx[i][1] * sin(angle), vtx[i][0] * sin(angle) + vtx[i][1] * cos(angle), vtx[i][2]);

			// translate
			vtx[i] += Vector3(1.85, 1.4, 0.75);
		}

		////set up shader
		mesh_obj->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("fox_map"));
		
		////set up texture
		mesh_obj->Add_Texture("tex_albedo", OpenGLTextureLibrary::Get_Texture("fox_albedo"));
		mesh_obj->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("fox_normal"));
		Set_Polygon_Mode(mesh_obj,PolygonMode::Fill);
		Set_Shading_Mode(mesh_obj,ShadingMode::Texture);
		
		////initialize
		mesh_obj->Set_Data_Refreshed();
		mesh_obj->Initialize();	
		mesh_object_array.push_back(mesh_obj);
		return (int)mesh_object_array.size()-1;
	}

	//// Generate L-system string
	// Applies a set of rules on an axiom a given number of times (i.e. the 'cycles' parameter).
	// A rule is defined as a list where the 0th entry is the letter being changed and 
	// the entries after that are what the letter can be changed to. If a rule specifies that a letter
	// can change to multiple things (i.e. it has multiple entries after the initial entry) 
	// then one of these things is randomly selected. Also has a choice an option "big" used to switch
	// between tree l-systems, currently only supports two but is expandable.
	std::string generate_L_system_string(int cycles, int big) {
		std::string axiom;	// starting axiom
		std::vector<std::vector<std::string>> rules; // rules for the L-system

		// decide which l-system to use
		// Letters' Meaning:
		// B: create a branch in the forward direction
		// L: create a leaf in the forward direction
		// X and x: rotate the forward direction along the x-axis
		// Y and y: rotate the forward direction along the y-axis
		// Z and z: rotate the forward direction along the z-axis
		// S and s: scale branches made from now on up/down
		// r: reset the scaling done so far
		// []: use to push and pop from the stack 
		// every other letter has no meaning
		if (!big) {
			// axiom and rule set for smaller tree
			axiom = "SSSSSSSSSSSP";
			rules = { {"B", "BB"}, {"P", "B[yXB][yyyxxxP]XBxB[YYYxxxxP]BP", "B[YxB][YYYXXXXXXP]XBxB[XXXyyyyP]BP"} };
		}
		else {
			// axiom and rule set for bigger tree
			axiom = "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBsBBBBBBBBBBBBBsBBBBBBBBBBBBBBBBBsBBBBBBBBsBBBBBBBBBBBBBBBBBBBBBBBBBBsBBBBBBBBBBBsBBBBBBBL";
			rules = {{"J","S"},
					{"L", "sBBBsJBBBBBBsJBBBBBBsJBBBBRBBBBsJBBBBBBBsJBBBBBBBsJBBBRBBBBsJBBYBsJBBBBsJBBsBBByBsJBBBBs[L][ssxxxxxxxxxYYYYYYYYYYYYYYYYYYYYYYYYL][ssXXXyyyyyyyyyyyyyyL][ssyyxxxxxxxL][ssYYXXXXXXXXL]"},
			{"R", "[sxxxxxxxxxYYYYYYYYYYYYYYYYYYYYYYYYBBsJBBsJBsJBBsJBBBBsJBsJBBsJBsBsJBBsJBBsJBssJsJsJBL][sXXXyyyyyyyyyyyyyyBBsJBsJBBsJBBBsJBBsJBsJBBsJBBsBBsJBsJBsJBsJsJBsJsJBsL][syyxxxxxxxBsJBBsBsJBBsBsJBBsJBsJBsJBBsJBsJBsJsJBsJsJBsL][sYYXXXXXXXXBBsJBBsJBsJBBsBsJBBsJBsJBsJBBsBBsJBBsJBsJBsJsJBsL]"}};
		}
		// apply the rules to the axiom a given number of times
		for (int i = 0; i < cycles; i++) {
			std::string next_axiom = "";

			// loop through letters in axiom
			for (int j = 0; j < axiom.length(); j++) {
				bool changed = false;
				char current_letter = axiom.at(j);

				// loop through rules
				for (int k = 0; k < rules.size(); k++) {

					// does this letter have rules
					if (current_letter==rules[k][0].at(0)) {
						changed = true;
						// if so, change it to one of its rules randomly
						next_axiom += rules[k][rand() % (rules[k].size() - 1) + 1];
						break;
					}
				}
				// if letter was not changed then leave it the same
				if (!changed) {
					next_axiom += current_letter;
				}
			}
			axiom = next_axiom;
		}
		return axiom;
	}

	//// Generates a tree mesh (two meshes: a mesh for branches and one for leaves) based on the L-systems from the function above this
	//// vertex is the beginning position the tree will be made at
	//// cycles is how many generations/iterations of the L-system chosen will be run
	//// big is 0 or 1, 0 is for an L-system that generates a skinny tree with no leaves, 
	//// 1 generates a thicker tree with lots of leaves
	int generate_Tree_Mesh(Vector3 vertex, int cycles, int big)
	{
		// mesh for branches 
		auto mesh_obj = Add_Interactive_Object<OpenGLTriangleMesh>();
		auto& mesh = mesh_obj->mesh;
		std::vector<Vector3>& vertices = mesh_obj->mesh.Vertices();	// vertex positions
		std::vector<Vector3i>& elements = mesh_obj->mesh.Elements();// mesh elements
		std::vector<Vector4f>& vtx_color = mesh_obj->vtx_color;		// vertex colors
		std::vector<Vector3> ori_cylin_vtx;   ////vertex array for cyclinder at origin
		std::vector<Vector3i> ori_cylin_tri;	////element array for cyclinder at origin
		std::vector<Vector3> ori_rect_vtx;   ////vertex array for rectangle at origin
		std::vector<Vector3i> ori_rect_tri;	////element array for rectangle at origin

		// creating a mesh for leaves
		auto mesh_obj_leaf = Add_Interactive_Object<OpenGLTriangleMesh>();
		auto& mesh_leaf = mesh_obj_leaf->mesh;
		std::vector<Vector3>& l_vertices = mesh_obj_leaf->mesh.Vertices();	// vertex positions
		std::vector<Vector3i>& l_elements = mesh_obj_leaf->mesh.Elements();    // mesh elements
		std::vector<Vector2>& l_uv = mesh_obj_leaf->mesh.Uvs();

		// creating low-resolution cyclinder used for branch mesh
		// first the vertices of the mesh
		for (float theta = PI / 3.; theta <= 2. * PI; theta += PI / 3.) {
			ori_cylin_vtx.push_back({ (1. / 3.)*(cos(theta)),(1. / 3.)*(sin(theta)),0. });
			ori_cylin_vtx.push_back({ (1. / 3.)*(cos(theta)),(1. / 3.)*(sin(theta)),1. });
		}
		// then the triangles
		for (int i = 0; i < ori_cylin_vtx.size(); i += 2) {
			ori_cylin_tri.push_back({ i, i + 1, (i + 2) % 12 });
			if (i == 0) {
				ori_cylin_tri.push_back({ i, 11, i + 1 });
			}
			else {
				ori_cylin_tri.push_back({ i, i - 1, i + 1 });
			}
		}

		// creating rectangle mesh used for leaves
		ori_rect_vtx = { 30.*Vector3(-1. / 4., 0., 0.), 30.*Vector3(1. / 4., 0., 0.), 30.*Vector3(-1. / 4., 0., 1.), 30.*Vector3(1. / 4., 0., 1.) };
		ori_rect_tri = { Vector3i(0,3,1), Vector3i(0,2,3) };

		// generating a L-system string
		std::string axiom = generate_L_system_string(cycles, big);

		// matrix used to rotate and translate the original cyclinder/rectangle
		Matrix4f transform;
		transform << 1., 0., 0., (float)vertex[0], 0., 1., 0., (float)vertex[1], 0., 0., 1., (float)vertex[2], 0., 0., 0., 1.;

		// the location after moving in the forward direction by 1
		Vector4f  target = Vector4f((float)vertex[0], (float)vertex[1], (float)vertex[2] + 1, 1.);
		bool rotated = false;	// did the forward direction get rotated at all
		float scale = 1;		// current scale of branches
		std::vector<Matrix<float, 5>> storage; // stack keeping track of place in commands

		// loop through letters in axiom
		for (int i = 0; i < axiom.length(); i++) {

			switch (axiom.at(i)) {
			// create a branch mesh
			case 'B':
			{
				if (rotated) {
					// calculating rotation matrix for mesh
					Vector3f target_vector = (target - transform.col(3)).head<3>();
					target_vector.normalize();
					Vector3f rotation_axis = Vector3f(-target_vector[1], target_vector[0], 0.);
					rotation_axis.normalize();

					// angle between two unit vectors is acos of dot product
					float angle = acos(target_vector[2]);

					// rotation about an arbitrary axis 
					Matrix4f m_rotation;
					m_rotation << cos(angle) + rotation_axis[0] * rotation_axis[0] * (1 - cos(angle)), rotation_axis[0] * rotation_axis[1] * (1 - cos(angle)),
						rotation_axis[1] * sin(angle), 0., rotation_axis[1] * rotation_axis[0] * (1 - cos(angle)), cos(angle) + rotation_axis[1] * rotation_axis[1] * (1 - cos(angle)),
						-rotation_axis[0] * sin(angle), 0., -rotation_axis[1] * sin(angle), rotation_axis[0] * sin(angle), cos(angle), 0., 0., 0., 0., 1.;
					
					// set new transform matrix (and target to make sure it is exactly one away from the current starting point)
					m_rotation.col(3) = transform.col(3);
					transform = m_rotation;
					target = Vector4f(transform.col(3)[0] + target_vector[0], transform.col(3)[1] + target_vector[1], transform.col(3)[2] + target_vector[2], 1.);
				}

				// modify the original cyclinder based on transformations so far and add it to the branch mesh
				for (int j = 0; j < ori_cylin_vtx.size(); j++) {
					// transform vertex and add to mesh
					Vector4f vtx = Vector4f((float)ori_cylin_vtx[j][0] * scale, (float)ori_cylin_vtx[j][1] * scale, (float)ori_cylin_vtx[j][2], 1.);
					vtx = transform * vtx;

					vertices.push_back(Vector3(vtx[0], vtx[1], vtx[2]));
				}
				// add triangle relations to mesh
				for (int j = 0; j < ori_cylin_tri.size(); j++) {
					int index_offset = (int)vertices.size() - 12;
					elements.push_back(Vector3i(ori_cylin_tri[j][0] + (index_offset), ori_cylin_tri[j][1] + (index_offset), ori_cylin_tri[j][2] + (index_offset)));
				}
				// updating target and transform matrix
				Vector4f temp = target;
				target = target - transform.col(3);
				target[3] = 0.;
				target.normalize();
				target = temp + target;
				transform.col(3) = temp;
				rotated = false;	
			}
			break;

			// create a leaf mesh
			case 'L':
			{
				if (rotated) {
					// calculating rotation matrix for mesh
					Vector3f target_vector = (target - transform.col(3)).head<3>();
					target_vector.normalize();
					Vector3f rotation_axis = Vector3f(-target_vector[1], target_vector[0], 0.);
					rotation_axis.normalize();

					// angle between two unit vectors is acos of dot product
					float angle = acos(target_vector[2]);

					// rotation about an arbitrary axis 
					Matrix4f m_rotation;
					m_rotation << cos(angle) + rotation_axis[0] * rotation_axis[0] * (1 - cos(angle)), rotation_axis[0] * rotation_axis[1] * (1 - cos(angle)),
						rotation_axis[1] * sin(angle), 0., rotation_axis[1] * rotation_axis[0] * (1 - cos(angle)), cos(angle) + rotation_axis[1] * rotation_axis[1] * (1 - cos(angle)),
						-rotation_axis[0] * sin(angle), 0., -rotation_axis[1] * sin(angle), rotation_axis[0] * sin(angle), cos(angle), 0., 0., 0., 0., 1.;

					// modify the original rectangle based on transformations so far and add it to the leaves mesh
					m_rotation.col(3) = transform.col(3);
					transform = m_rotation;
					target = Vector4f(transform.col(3)[0] + target_vector[0], transform.col(3)[1] + target_vector[1], transform.col(3)[2] + target_vector[2], 1.);
				}

				for (int j = 0; j < ori_rect_vtx.size(); j++) {
					// transform vertex and add to mesh
					Vector4f vtx = Vector4f((float)ori_rect_vtx[j][0], (float)ori_rect_vtx[j][1], (float)ori_rect_vtx[j][2], 1.);
					vtx = transform * vtx;
					l_vertices.push_back(Vector3(vtx[0], vtx[1], vtx[2]));
				}
				// add triangle relations to mesh
				for (int j = 0; j < ori_rect_tri.size(); j++) {
					int index_offset = (int)l_vertices.size() - 4;
					l_elements.push_back(Vector3i(ori_rect_tri[j][0] + (index_offset), ori_rect_tri[j][1] + (index_offset), ori_rect_tri[j][2] + (index_offset)));
				}
				// updating target and transform matrix
				Vector4f temp = target;
				target = target - transform.col(3);
				target[3] = 0.;
				target.normalize();
				target = temp + target;
				transform.col(3) = temp;
				rotated = false;

				// adding uvs for leaves mesh, same for each individual leaf, the uvs are based on the maple leaf texture
				l_uv.push_back(Vector2(.5, 1.));
				l_uv.push_back(Vector2(1., 1.));
				l_uv.push_back(Vector2(.5, 0.));
				l_uv.push_back(Vector2(1., 0.));

			}
			break;

			// rotate left of x-axis
			case 'X':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << 1., 0., 0., 0.,
					0., cos(angle), -sin(angle), 0.,
					0., sin(angle), cos(angle), 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;
			// rotate right of x-axis
			case 'x':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << 1., 0., 0., 0.,
					0., cos(angle), sin(angle), 0.,
					0., -sin(angle), cos(angle), 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;
			// rotate left of y-axis
			case 'Y':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << cos(angle), 0., sin(angle), 0.,
					0., 1., 0., 0.,
					-sin(angle), 0., cos(angle), 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;
			// rotate right of y-axis
			case 'y':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << cos(angle), 0., -sin(angle), 0.,
					0., 1., 0., 0.,
					sin(angle), 0., cos(angle), 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;

			// rotate left of Z-axis
			case 'Z':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << cos(angle), sin(angle), 0., 0.,
					-sin(angle), cos(angle), 0., 0.,
					0., 0., 0., 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;
			// rotate right of z-axis
			case 'z':
			{
				// rotating transform matrix and target by a random angle within the range [20,60] degrees.
				float angle = (float)(rand() % 20 + 10) * ((float)PI / 180.f);
				Matrix4f rotation;
				rotation << cos(angle), -sin(angle), 0., 0.,
					sin(angle), cos(angle), 0., 0.,
					0., 0., 0., 0.,
					0., 0., 0., 1.;

				// translate, rotate, translate target to get uniform rotation 
				Matrix4f m1;
				m1 << 1.f, 0.f, 0.f, -target[0], 0.f, 1.f, 0.f, -target[1], 0.f, 0.f, 1.f, -target[2] + 1.f, 0.f, 0.f, 0.f, 1.f;
				Matrix4f m2;
				m2 << 1.f, 0.f, 0.f, target[0], 0.f, 1.f, 0.f, target[1], 0.f, 0.f, 1.f, target[2] - 1.f, 0.f, 0.f, 0.f, 1.f;

				// apply translations and rotation to target
				target = m2 * rotation*m1*target;
				rotated = true;
			}
			break;

			// scale down object
			case 's':
			{
				scale *= .9f;
			}
			break;

			// scale up object
			case 'S':
			{
				scale *= 1.1f;
			}
			break;
			// reset scale factor
			case 'r':
			{
				scale = 1;
			}
			break;
			// push current transform matrix, scale factor and target on stack
			case '[':
			{
				Matrix<float, 5> track;
				for (int i = 0; i < 4; i++) {
					track.col(i).head<4>() = transform.col(i);
				}
				track.col(4).head<4>() = target;
				track.col(0)[4] = scale;
				storage.push_back(track);
			}
			break;
			// get a transform matrix, scale factor and target from stack
			case ']':
			{
				Matrix<float, 5> track = storage.back();
				storage.pop_back();
				target = track.col(4).head<4>();

				for (int i = 0; i < 4; i++) {
					transform.col(i) = track.col(i).head<4>();
				}
				scale= track.col(0)[4];
			}
			break;
			}
		}

		// shrink models so they fit in the scene properly
		for (int vtx = 0; vtx < vertices.size(); vtx++) {
			vertices[vtx] = .01*vertices[vtx];
		}
		for (int vtx = 0; vtx < l_vertices.size(); vtx++) {
			l_vertices[vtx] = .01*l_vertices[vtx];
		}

		////setting up spherical cordinates for branches mesh uvs
		std::vector<Vector2>& uv = mesh_obj->mesh.Uvs();
		Update_Uv_Using_Spherical_Coordinates(vertices, uv);

		////set up the leaves/branches if they were created
		if (vertices.size() > 0) {
			////set up texture
			mesh_obj->Add_Texture("tex_albedo", OpenGLTextureLibrary::Get_Texture("bark_albedo"));
			mesh_obj->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("bark_normal"));
			Set_Polygon_Mode(mesh_obj, PolygonMode::Fill);
			Set_Shading_Mode(mesh_obj, ShadingMode::Texture);
			////set up shader
			mesh_obj->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("tree_bark"));

			////initialize
			mesh_obj->Set_Data_Refreshed();
			mesh_obj->Initialize();
			mesh_object_array.push_back(mesh_obj);

		}
		if (l_vertices.size() > 0) {
			//set up shader
			mesh_obj_leaf->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("leaves"));

			//set up texture
			mesh_obj_leaf->Add_Texture("tex_albedo", OpenGLTextureLibrary::Get_Texture("leaves_albedo"));
			mesh_obj_leaf->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("leaves_normal"));
			Set_Polygon_Mode(mesh_obj_leaf, PolygonMode::Fill);
			Set_Shading_Mode(mesh_obj_leaf, ShadingMode::Texture);

			//initialize
			mesh_obj_leaf->Set_Data_Refreshed();
			mesh_obj_leaf->Initialize();
			mesh_object_array.push_back(mesh_obj_leaf);
		}
		return (int)mesh_object_array.size() - 1;
	}

	//// Generates a number of trees within a given plane (given using the normal and a starting point vertex)
	//// the point given in the plane is also the starting point that the algorithm starts generating trees at.
	//// normal is the normal to the plane
	//// vertex is the starting point/point within the plane
	//// num_trees is the number of trees to be generated
	//// cycles_min and cycles_max determine the maximum and minimum amount of iterations/generations for the L-system 
	//// used to generate trees
	 void generate_Forest(Vector3 normal, Vector3 vertex, int num_trees, int cycles_min, int cycles_max, int big) {
		 // generate random list of coordinates within plane
		 std::vector<Vector3> random_cords;
		 for (int i = -1000; i < 1000; i+=100) {
			 for (int j = -1000; j < 1000; j+=100) {
				 random_cords.push_back(Vector3(i+vertex[0], j+vertex[1], 0));
			 }
		 }
		 // shuffle the list 
		 std::shuffle(begin(random_cords), end(random_cords), std::default_random_engine());

		 // hard coded maximum number of trees
		 if (num_trees > 100) {
			 num_trees = 100;
		 }

		 // generate trees based on the shuffled list
		 for (int i = 0; i < num_trees; i++) {
			 // set z-coordinate to be within plane
			 random_cords[i][2] = ((normal[0]*(random_cords[i][0]-vertex[0])+normal[1]*(random_cords[i][1] - vertex[1]))/(-normal[2]))+vertex[2];

			 // generate tree at coordinate;
			 generate_Tree_Mesh(random_cords[i], rand()%((cycles_max-cycles_min)+1)+cycles_min, big);
		 }
	}

	virtual void Initialize_Data()
	{
		Add_Shaders();
		Add_Textures();
		Add_Background();
		Add_Fox();
		//add the plane mesh object for snow terrain
		int obj_idx = Add_Obj_Mesh_Object("plane.obj");
		auto plane_obj = mesh_object_array[obj_idx];

		////set up texture
		plane_obj->Add_Texture("tex_albedo", OpenGLTextureLibrary::Get_Texture("plane_albedo"));
		plane_obj->Add_Texture("tex_normal", OpenGLTextureLibrary::Get_Texture("plane_normal"));
		Set_Polygon_Mode(plane_obj, PolygonMode::Fill);
		Set_Shading_Mode(plane_obj, ShadingMode::Texture);

		// add plane
		plane_obj->Add_Shader_Program(OpenGLShaderLibrary::Get_Shader("snow"));
		Set_Polygon_Mode(plane_obj, PolygonMode::Fill);
		Set_Shading_Mode(plane_obj, ShadingMode::Texture);
		plane_obj->Set_Data_Refreshed();
		plane_obj->Initialize();

		// generating fox forest scene, the order is important here because of alpha blending 
		generate_Tree_Mesh({-530,160,0 }, 4, 1);
		generate_Tree_Mesh({-100,80,35}, 5, 0);
		generate_Forest({ 0,0,1 }, { 0,0, 30}, 30, 4, 6, 0);
		generate_Tree_Mesh({ 100,200,32 }, 4, 1);

		// randomly generating a forest, this takes a long time so uncomment at your own risk
		//generate_Forest({ 0,0,1 }, { 3000,3000, 30 }, 30, 4, 4, 0);
		//generate_Forest({ 0,0,1 }, { 4000,3000, 30 }, 30, 4, 4, 0);
		//generate_Forest({ 0,0,1 }, { 3000,4000, 30 }, 30, 4, 4, 0);
		//generate_Forest({ 0,0,1 }, { 4000,4000, 30 }, 30, 4, 4, 0);
		//generate_Forest({ 0,0,1 }, { 3000,3000, 30 }, 8, 4, 4, 1);
		//generate_Forest({ 0,0,1 }, { 3000,4000, 30 }, 8, 4, 4, 1);
		//generate_Forest({ 0,0,1 }, { 4000,3000, 30 }, 8, 4, 4, 1);
		//generate_Forest({ 0,0,1 }, { 4000,4000, 30 }, 8, 4, 4, 1);
	}

	////Goto next frame 
	virtual void Toggle_Next_Frame()
	{
		
		for (auto& mesh_obj : mesh_object_array) {
			mesh_obj->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
			opengl_background->setTime(GLfloat(clock() - startTime) / CLOCKS_PER_SEC);
		}

		OpenGLViewer::Toggle_Next_Frame();
	}

	virtual void Run()
	{
		OpenGLViewer::Run();
	}
};

int main(int argc,char* argv[])
{
	FinalProjectDriver driver;
	driver.Initialize();
	driver.Run();	
}

#endif