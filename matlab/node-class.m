classdef PCMC_Node < handle
% Node class for a 3-dimensional framed structure
    
    % Private properties go here
    properties (Access = private)
        % 3x1 vector containing the x, y, and z coordinates of the node
        node_coord
        node_number
        node_dof
    end
    
    % Public methods go here
    methods (Access = public)
        %% Constructor
        %  Replace XYZ by your initials before proceeding
        %    Arguments
        %      node_coord:  3x1 vector containing the x, y, and z coordinates of the node
        function self = PCMC_Node(node_coord,node_number)
            self.node_coord = node_coord;
            self.node_number = node_number;
            self.node_dof = AssignDOF(self);
        end
        
        %% Getter Functions %%
        %% Get Node Coordinates
        function node_coord = GetNodeCoord(self)
            node_coord = self.node_coord;
        end
        
        %% Get Node Num
        function node_number = GetNodeNumber(self)
            node_number = self.node_number;
        end
        
        %% Get Node DOF
        function node_dof = GetNodeDOF(self)
            node_dof = self.node_dof;
        end
      
    end
    
    % Private methods go here
    methods (Access = private)
        
        % Assign Degrees of Freedom (DOF)
        function node_dof = AssignDOF(self)
            node_dof = [6*self.node_number-5 6*self.node_number-4 6*self.node_number-3 6*self.node_number-2 6*self.node_number-1 6*self.node_number]';
        end
    end
end
